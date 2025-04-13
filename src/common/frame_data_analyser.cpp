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

// Static structs
struct game_state frame_data_analyser::m_state = {};
struct game_state frame_data_analyser::m_previous_state = {};
struct player_attack_frames frame_data_analyser::m_attack_frames = {};

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

void frame_data_analyser::update_attack_init_frames() {
    // Check if P1 initiated attack
    if (is_attack(m_state.p1_intent) &&
        m_previous_state.p1_intent != m_state.p1_intent) {
        m_attack_frames.p1_last_attack_frame = m_state.game_frame;
    }

    // Check if P2 initiated attack
    if (is_attack(m_state.p2_intent) &&
        m_previous_state.p2_intent != m_state.p2_intent) {
        m_attack_frames.p2_last_attack_frame = m_state.game_frame;
    }
}

bool frame_data_analyser::has_new_connection() {
    return (!m_previous_state.p1_connection && m_state.p1_connection) ||
            (!m_previous_state.p2_connection && m_state.p2_connection);

}

bool frame_data_analyser::loop() {
    uint32_t current_frame = current_game_frame();
    if (current_frame == READ_ERROR) {
        log_fatal("failed to read game's current frame number");
        return false;
    }

    // Check if the analyser is in sync with the game
    if (m_previous_state.game_frame != current_frame - 1 && m_previous_state.game_frame != 0) {
        int64_t frames_off = (int64_t) m_previous_state.game_frame - (int64_t) current_frame;
        // Analyser is ahead skip this tick
        if (frames_off == 0) {
            return true;
        }

        log_warn("analyser is off by \"%lld\" frames", frames_off);
    }

    // Read the game's state
    if (read_game_state(&m_state) != 0) {
        log_fatal("failed to read game's state");
        return false;
    }

    update_attack_init_frames();

    if (has_new_connection()) {
        // P1 initiated attack
        int32_t startup_frames;
        if (m_attack_frames.p1_last_attack_frame > m_attack_frames.p2_last_attack_frame) {
            startup_frames = m_state.game_frame - m_attack_frames.p1_last_attack_frame;
        // P2 initiated attack
        } else {
            startup_frames = m_state.game_frame - m_attack_frames.p2_last_attack_frame;
        }

        const int32_t frame_advantage = startup_frames - (m_state.p1_recovery_frames - m_state.p2_recovery_frames);

        struct frame_data_point data_point = {startup_frames, frame_advantage};

        callback(data_point);
    }

    // Store last state
    m_previous_state = m_state;
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

    if (read_game_state(&m_state) != 0) {
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
