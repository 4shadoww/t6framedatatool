#include <chrono>
#include <cstdint>
#include <thread>

extern "C" {
#include "logging.h"
#include "memory_reader.h"
}

#include "frame_data_analyser.hpp"

// Constants

// Run the tool twice as fast as the game to get accurate measurements
#define TICK_LENGTH 8333333

// Five seconds of frames
#define FRAME_BUFFER_SIZE 60 * 5
#define PLAYER_ACTION_BUFFER_SIZE 5

//// frame_data_analyser
ring_buffer<game_state> frame_data_analyser::m_frame_buffer(FRAME_BUFFER_SIZE);
ring_buffer<start_frame> frame_data_analyser::m_p1_start_frames(PLAYER_ACTION_BUFFER_SIZE);
ring_buffer<start_frame> frame_data_analyser::m_p2_start_frames(PLAYER_ACTION_BUFFER_SIZE);

void (*frame_data_analyser::callback)(struct frame_data_point);

bool frame_data_analyser::is_attack(const int intent) {
    switch (intent) {
    case player_intents::ATTACK1:
    case player_intents::ATTACK3:
    case player_intents::ATTACK5:
    case player_intents::ATTACK7:
        return true;

    case player_intents::IDLE:
    case player_intents::INPUT_BUFFERING:
    case player_intents::BLOCK:
    case player_intents::WALK:
    case player_intents::SIDE_STEP:
    case player_intents::DOUBLE_SIDE_STEP:
    case player_intents::STASIS:
    case player_intents::GRAP_INIT:
    case player_intents::GRAP_CONNECT:
        return false;

    default:
        log_warn("unknown player intent \"%i\"", intent);
        return false;
    }
}

void frame_data_analyser::analyse_start_frames() {
    game_state *current = m_frame_buffer.head();
    game_state *previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return;
    }

    // Check if P1 initiated attack
    if (is_attack(current->p1_intent) &&
        previous->p1_intent != current->p1_intent) {
        m_p1_start_frames.push( {m_frame_buffer.head_index(), current->game_frame});
    }

    // Check if P2 initiated attack
    if (is_attack(current->p2_intent) &&
        previous->p2_intent != current->p2_intent) {
        m_p2_start_frames.push( {m_frame_buffer.head_index(), current->game_frame});
    }
}

connection_event frame_data_analyser::has_new_connection() {
    game_state *current = m_frame_buffer.head();
    game_state *previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return NO_CONNECTION;
    }

    if (!previous->p1_connection && current->p1_connection) {
        return P1_CONNECTION;
    } else if (!previous->p2_connection && current->p2_connection) {
        return P2_CONNECTION;
    }

    return NO_CONNECTION;
}

void frame_data_analyser::handle_connection() {
    connection_event connection = has_new_connection();
    if (connection == NO_CONNECTION) {
        return;
    }

    start_frame start;

    switch (connection) {
    case NO_CONNECTION:
        return;
    case P1_CONNECTION:
        m_p2_start_frames.clear();
        start = m_p1_start_frames.pop();
        break;
    case P2_CONNECTION:
        m_p1_start_frames.clear();
        start = m_p2_start_frames.pop();
        break;
    }

    if (start.game_frame == 0) {
        log_error("Player start up buffer has invalid state");
        return;
    }

    const game_state *current = m_frame_buffer.head();
    const int32_t startup_frames = current->game_frame - start.game_frame;

    const int32_t frame_advantage = startup_frames - (current->p1_recovery_frames - current->p2_recovery_frames);

    struct frame_data_point data_point = {startup_frames, frame_advantage};

    callback(data_point);
}

bool frame_data_analyser::update_game_state() {
    // Read the game's state
    game_state state;
    if (read_game_state(&state) != 0) {
        return false;
    }

    m_frame_buffer.push(state);

    return true;
}

bool frame_data_analyser::loop() {
    // Analyser timing
    uint32_t current_frame = current_game_frame();
    if (current_frame == READ_ERROR) {
        log_fatal("failed to read game's current frame number");
        return false;
    }

    game_state *previous = m_frame_buffer.head();

    // Check if the analyser is in sync with the game
    if (previous->game_frame != current_frame - 1 && previous->game_frame != 0) {
        int64_t frames_off = (int64_t) previous->game_frame - (int64_t) current_frame;
        // Analyser is ahead skip this tick
        if (frames_off == 0) {
            return true;
        }

        log_warn("analyser is off by \"%lld\" frames", frames_off);
    }

    // Analysis logic
    if (!update_game_state()) {
        log_fatal("failed to read game's state");
        return false;
    }

    analyse_start_frames();
    handle_connection();

    return true;
}

bool frame_data_analyser::init(void (*callback)(struct frame_data_point)) {
    if (callback == nullptr) {
        return false;
    }
    frame_data_analyser::callback = callback;

    const int result = init_memory_reader();
    if (result != MR_INIT_OK) {
        return false;
    }

    // Analysis logic
    if (!update_game_state()) {
        log_fatal("failed to read game's state");
        return false;
    }

    return true;
}

bool frame_data_analyser::start(void (*callback)(struct frame_data_point)) {
    if (!init(callback)) {
        return false;
    }

    // Main loop
    while (true) {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();

        if (!loop()) {
            // Unrecoverable error has occurred
            return false;
        }

        // Wait until next tick
        auto delta = end - start;
        while (std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count() < TICK_LENGTH) {
            // Sleep for a bit to save CPU time
            std::this_thread::sleep_for (std::chrono::nanoseconds(delta / 2));

            end = std::chrono::high_resolution_clock::now();
            delta = end - start;
        }
    }

    return true;
}
