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

struct game_state frame_data_analyser::m_state;
uint32_t frame_data_analyser::m_last_frame = 0;
uint32_t frame_data_analyser::m_last_connection = 0;
void (*frame_data_analyser::callback)(struct frame_data_point);

bool frame_data_analyser::has_new_connection() {
    return m_state.game_frame - m_last_connection  != m_state.p1_frames_last_connection;

}

void frame_data_analyser::update_last_connection() {
    m_last_connection = m_state.game_frame - m_state.p1_frames_last_connection;
}

bool frame_data_analyser::loop() {
    uint32_t current_frame = current_game_frame();
    if (current_frame == -1) {
        log_fatal("failed to read game's current frame number");
        return false;
    }

    // Check if the analyser is in sync with the game
    if (m_last_frame != current_frame - 1 && m_last_frame && m_last_frame != 0) {
        int64_t frames_off = (int64_t) m_last_frame - (int64_t) current_frame;
        // Analyser is ahead skip this tick
        if (frames_off == 0) {
            return true;
        }

        log_warn("analyser is off by \"%lld\" frames", frames_off);
        m_last_frame = current_frame;
    }

    // Read the game's state
    if (read_game_state(&m_state) != 0) {
        log_fatal("failed to read game's state");
        return false;
    }

    m_last_frame = m_state.game_frame;

    if (has_new_connection()) {
        m_last_connection = m_state.game_frame - m_state.p1_frames_last_connection;
        int32_t move_frames = m_state.p1_frames_last_action - m_state.p1_frames_last_connection;
        int32_t frame_advantage = (m_state.p2_recovery_frames - m_state.p2_frames_last_action) -
                                  (m_state.p1_recovery_frames - m_state.p1_frames_last_action);

        struct frame_data_point data_point = {move_frames, frame_advantage};

        callback(data_point);
    }
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

    // Set up initial values
    update_last_connection();

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
