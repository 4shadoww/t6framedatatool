#include "ring_buffer.hpp"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <thread>

#include "logging.h"
#include "memory_reader.h"

#include "frame_data_analyser.hpp"

// Constants

// Run the tool twice as fast as the game to get accurate measurements
#define TICK_LENGTH 8333333

// Five seconds of frames
#define FRAME_BUFFER_SIZE 60 * 5
#define PLAYER_ACTION_BUFFER_SIZE 5
#define REVERSE_WALK_LIMIT 6

//// frame_data_analyser
///
bool frame_data_analyser::m_stop = false;
ring_buffer<game_state> frame_data_analyser::m_frame_buffer(FRAME_BUFFER_SIZE);
ring_buffer<start_frame> frame_data_analyser::m_p1_start_frames(PLAYER_ACTION_BUFFER_SIZE);
ring_buffer<start_frame> frame_data_analyser::m_p2_start_frames(PLAYER_ACTION_BUFFER_SIZE);

event_listener *frame_data_analyser::m_listener = nullptr;

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
    case player_intents::FALLING:
    case player_intents::LANDING:
    case player_intents::STASIS:
    case player_intents::WHIFF:
    case player_intents::GRAP_INIT:
    case player_intents::GRAP_CONNECT:
        return false;

    default:
        log_warn("unknown player intent \"%i\"", intent);
        return false;
    }
}

void frame_data_analyser::analyse_start_frames() {
    const game_state * const current = m_frame_buffer.head();
    const game_state * const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return;
    }

    // Check if P1 initiated attack
    if (is_attack(current->p1_intent) &&
        previous->p1_intent != current->p1_intent) {
        m_p1_start_frames.push({m_frame_buffer.head_index(),
                                current->p1_recovery_frames,
                                current->game_frame});
    }

    // Check if P2 initiated attack
    if (is_attack(current->p2_intent) &&
        previous->p2_intent != current->p2_intent) {
        m_p2_start_frames.push({m_frame_buffer.head_index(),
                                current->p2_recovery_frames,
                                current->game_frame});
    }
}

connection_event frame_data_analyser::has_new_connection() {
    const game_state * const current = m_frame_buffer.head();
    const game_state * const previous = m_frame_buffer.get_from_head(1);

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

start_frame frame_data_analyser::get_startup_frame(const bool p2) {
    ring_buffer<start_frame> *buffer = p2 ? &m_p2_start_frames : &m_p1_start_frames;

    const size_t item_count = buffer->item_count();
    for (size_t i = 0; i < item_count; i++) {
        const start_frame start_frame = buffer->pop();
        // Reverse walk frames
        for (size_t j = 0; j < REVERSE_WALK_LIMIT; j++) {
            game_state *frame = m_frame_buffer.get_from_head(j);
            const uint32_t last_action = p2 ? frame->p2_frames_last_action : frame->p1_frames_last_action;

            if (frame->game_frame - start_frame.game_frame + 1 == last_action) {
                return start_frame;
            }
        }
    }

    log_fatal("plater startup frame not found");
    return {};
}


void frame_data_analyser::handle_connection() {
    const connection_event connection = has_new_connection();
    if (connection == NO_CONNECTION) {
        return;
    }

    const game_state * const current = m_frame_buffer.head();

    start_frame start;
    uint32_t opponent_recovery_frames;

    switch (connection) {
    case NO_CONNECTION:
        return;
    case P1_CONNECTION:
        m_p2_start_frames.clear();
        opponent_recovery_frames = current->p2_recovery_frames;
        start = get_startup_frame(false);
        break;
    case P2_CONNECTION:
        m_p1_start_frames.clear();
        opponent_recovery_frames = current->p1_recovery_frames;
        start = get_startup_frame(true);
        break;
    }

    if (start.game_frame == 0) {
        log_error("Player start up buffer has invalid state");
        return;
    }

    int32_t startup_frames;
    int32_t frame_advantage;

    if (connection == P1_CONNECTION) {
        startup_frames = current->game_frame - start.game_frame;
        frame_advantage = startup_frames - (start.recovery_frames - opponent_recovery_frames);
    } else {
        startup_frames = current->game_frame - start.game_frame;
        frame_advantage = (start.recovery_frames - opponent_recovery_frames) - startup_frames;
        startup_frames = 0;
    }

    struct frame_data_point data_point = {startup_frames, frame_advantage};

    m_listener->frame_data(data_point);
}

float frame_data_analyser::calculate_distance(const game_state * const state) {
    return std::sqrt(
         std::pow(state->p1_position.x - state->p2_position.x, 2) +
         std::pow(state->p1_position.z - state->p2_position.z, 2)
    ) / 1000;
}

void frame_data_analyser::handle_distance() {
    const game_state * const current = m_frame_buffer.head();
    //log_info("TEST %f", current->p1_position.x);
    const float distance = calculate_distance(current);
    m_listener->distance(distance);
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
    const uint32_t current_frame = current_game_frame();
    if (current_frame == READ_ERROR) {
        log_fatal("failed to read game's current frame number");
        return false;
    }

    const game_state * const previous = m_frame_buffer.head();

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

    // Collect frames before analysis
    if (m_frame_buffer.item_count() < REVERSE_WALK_LIMIT) {
        return true;
    }

    analyse_start_frames();
    handle_connection();
    handle_distance();

    return true;
}

bool frame_data_analyser::init(event_listener *listener) {
    if (listener == nullptr) {
        return false;
    }
    frame_data_analyser::m_stop = false;
    frame_data_analyser::m_listener = listener;

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

bool frame_data_analyser::start(event_listener *listener) {
    if (!init(listener)) {
        return false;
    }

    // Main loop
    while (!m_stop) {
        auto start = std::chrono::high_resolution_clock::now();

        if (!loop()) {
            // Unrecoverable error has occurred
            return false;
        }
        auto end = std::chrono::high_resolution_clock::now();

        // Wait until next tick
        auto delta = end - start;
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count();
        while (nanos < TICK_LENGTH) {
            // Sleep for a bit to save CPU time
            std::this_thread::sleep_for (std::chrono::nanoseconds((TICK_LENGTH - nanos) / 2));

            end = std::chrono::high_resolution_clock::now();
            delta = end - start;
            nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count();
        }
    }

    return true;
}

void frame_data_analyser::stop() {
    m_stop = true;
}
