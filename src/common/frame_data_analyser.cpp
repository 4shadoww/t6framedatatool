#include <chrono>
#include <cstdint>

extern "C" {
#include "logging.h"
#include "memory_reader.h"
}

#include "frame_data_analyser.hpp"

// Constants
#define TICK_LENGTH 16

struct game_state frame_data_analyser::m_state;
uint32_t frame_data_analyser::m_last_frame = 0;
uint32_t frame_data_analyser::m_last_connection = 0;

bool frame_data_analyser::new_connection() {
    return m_state.game_frame - m_last_connection  != m_state.p1_frames_last_connection;

}

void frame_data_analyser::loop() {
    if (read_game_state(&m_state) != 0) {
        log_fatal("failed to read game's state");
        return;
    }

    // Check if the analyser is in sync with the game
    if (m_last_frame != m_state.game_frame - 1 && m_last_frame && m_last_frame != 0) {
        int64_t frames_off = (int64_t) m_last_frame - (int64_t) m_state.game_frame - 1 ;
        //log_warn("analyser is off by \"%lld\" frames", frames_off);

        m_last_frame = m_state.game_frame;

        // Analyser is ahead skip this tick
        if (frames_off > 0) {
            return;
        }
    }
    m_last_frame = m_state.game_frame;

    if (new_connection()) {
        m_last_connection = m_state.game_frame - m_state.p1_frames_last_connection;
        int32_t move_frames = m_state.p1_frames_last_action - m_state.p1_frames_last_connection;
        int32_t frame_advantage = (m_state.p2_recovery_frames - m_state.p2_frames_last_action) -
                                  (m_state.p1_recovery_frames - m_state.p1_frames_last_action);
        log_info("move frames: %d, frame advantage: %d", move_frames, frame_advantage);
    }
}

bool frame_data_analyser::start() {
    const int result = init_memory_reader();
    if (result != MR_INIT_OK) {
        return false;
    }

    // Main loop
    while (true) {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();

        loop();

        // Wait
        while (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < TICK_LENGTH) {
            end = std::chrono::high_resolution_clock::now();
        }
    }


    return true;
}
