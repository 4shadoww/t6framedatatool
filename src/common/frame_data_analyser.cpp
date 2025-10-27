/*
  Copyright (C) 2025 Noa-Emil Nissinen

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.    If not, see <https://www.gnu.org/licenses/>.
*/

#include "ring_buffer.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <thread>

#include "game_state_reader.h"
#include "logging.h"
#include "memory_reader_types.h"

#include "frame_data_analyser.hpp"

// Constants

// Run the tool twice as fast as the game to get accurate measurements
#define TICK_LENGTH 8333333

// Ten seconds of frames
#define FRAME_BUFFER_SIZE (size_t) (60 * 10)
#define PLAYER_ACTION_BUFFER_SIZE 5
#define REVERSE_WALK_LIMIT 30

//// frame_data_analyser
///
volatile bool FrameDataAnalyser::m_stop = false;
RingBuffer<game_state> FrameDataAnalyser::m_frame_buffer(FRAME_BUFFER_SIZE);
RingBuffer<StartFrame> FrameDataAnalyser::m_p1_start_frames(PLAYER_ACTION_BUFFER_SIZE);
RingBuffer<StartFrame> FrameDataAnalyser::m_p2_start_frames(PLAYER_ACTION_BUFFER_SIZE);

EventListener *FrameDataAnalyser::m_listener = nullptr;

// Avoid log spam
int FrameDataAnalyser::m_last_player_intent = 0;
bool FrameDataAnalyser::m_logging = false;

void FrameDataAnalyser::log_frame() {
    const game_state *const state = m_frame_buffer.head();
    std::stringstream stream;
    stream << "FRAME: " << state->game_frame << std::endl
           << "P1 last action: " << state->p1_frames_last_action << std::endl
           << "P1 recovery frames: " << state->p1_recovery_frames << std::endl
           << "P1 connection: " << (bool) state->p1_connection << std::endl
           << "P1 intent: " << state->p1_intent << std::endl
           << "P1 move: " << state->p1_move << std::endl
           << "P1 state: " << state->p1_state << std::endl
           << "P1 position: " << state->p1_position.x << ", " << state->p1_position.y << ", " << state->p1_position.z
           << std::endl
           << "P2 last action: " << state->p2_frames_last_action << std::endl
           << "P2 recovery frames: " << state->p2_recovery_frames << std::endl
           << "P2 connection: " << (bool) state->p2_connection << std::endl
           << "P2 intent: " << state->p2_intent << std::endl
           << "P2 move: " << state->p2_move << std::endl
           << "P2 state: " << state->p2_state << std::endl
           << "P2 position: " << state->p2_position.x << ", " << state->p2_position.y << ", " << state->p2_position.z;

    log_info(stream.str().c_str());
}

bool FrameDataAnalyser::is_attack(const PlayerIntent &intent) {
    switch (intent) {
    case PlayerIntent::ATTACK1:
    case PlayerIntent::ATTACK3:
    case PlayerIntent::ATTACK5:
    case PlayerIntent::ATTACK7:
        return true;

    case PlayerIntent::IDLE:
    case PlayerIntent::INPUT_BUFFERING:
    case PlayerIntent::BLOCK:
    case PlayerIntent::WALK:
    case PlayerIntent::SIDE_STEP:
    case PlayerIntent::DOUBLE_SIDE_STEP:
    case PlayerIntent::FALLING:
    case PlayerIntent::LANDING:
    case PlayerIntent::STASIS:
    case PlayerIntent::WHIFF:
    case PlayerIntent::GRAP_INIT:
    case PlayerIntent::GRAP_CONNECT:
    case PlayerIntent::SIDE_ROLLING:
        return false;

    default:
        log_warn("unknown player intent \"%i\"", intent);
        return false;
    }
}

bool FrameDataAnalyser::p1_recovery_reset(const game_state *const previous, const game_state *const current) {
    return previous->p1_move != current->p1_move;
}

bool FrameDataAnalyser::p2_recovery_reset(const game_state *const previous, const game_state *const current) {
    return previous->p2_move != current->p2_move;
}

const char *FrameDataAnalyser::player_status(const PlayerState state) {
    static const char STANDING[] = "Standing";
    static const char CROUCH[] = "Crouch";
    static const char CROUCHING[] = "Crouching";
    static const char JUMPING[] = "Jumping";
    static const char AIRBORNE[] = "Airborne";
    static const char GROUNDED[] = "Grounded";
    static const char UNDETERMINABLE[] = "Undeterminable";

    switch (state) {
    case PlayerState::STANDING:
    case PlayerState::MOVE_BACKWARDS:
    case PlayerState::MOVE_FORWARDS:
    case PlayerState::DASH_BACKWARDS:
    case PlayerState::DASH_FORWARDS:
    case PlayerState::MOVE:
    case PlayerState::RECOVER1:
    case PlayerState::RECOVER2:
    case PlayerState::SIDE_STEPPING:
    case PlayerState::STANDING_HIT:
    case PlayerState::CROUCH_DASH_JUMP:
        return STANDING;
    case PlayerState::CROUCH:
        return CROUCH;
    case PlayerState::CROUCHING:
    case PlayerState::CROUCHING_ATTACK:
    case PlayerState::CROUCHING_BACKWARDS:
    case PlayerState::CROUCHING_FORWARDS:
        return CROUCHING;
    case PlayerState::JUMPING:
    case PlayerState::JUMPING_FORWARDS:
    case PlayerState::JUMPING_BACKWARDS:
        return JUMPING;
    case PlayerState::AIRBORNE:
        return AIRBORNE;
    case PlayerState::GROUNDED1:
    case PlayerState::GROUNDED2:
        return GROUNDED;
    }

    if (m_last_player_intent != (int) state) {
        log_debug("unknown player status %d", state);
        m_last_player_intent = (int) state;
    }

    return UNDETERMINABLE;
}

bool FrameDataAnalyser::p1_initiated_attack(const game_state *previous, const game_state *current) {
    return ((PlayerMove) current->p1_move != PlayerMove::IDLE && previous->p1_move != current->p1_move);
}

bool FrameDataAnalyser::p2_initiated_attack(const game_state *previous, const game_state *current) {
    return ((PlayerMove) current->p2_move != PlayerMove::IDLE && previous->p2_move != current->p2_move);
}

void FrameDataAnalyser::analyse_start_frames() {
    const game_state *const current = m_frame_buffer.head();
    const game_state *const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return;
    }

    // Check if P1 initiated attack
    if (p1_initiated_attack(previous, current)) {
        m_p1_start_frames.push({.index = m_frame_buffer.head_index(),
                                .recovery_frames = current->p1_recovery_frames,
                                .game_frame = current->game_frame});
        if (m_logging) {
            log_info("MARK STARTUP P1: %i", current->game_frame);
        }
    }

    // Check if P2 initiated attack
    if (p2_initiated_attack(previous, current)) {
        m_p2_start_frames.push({.index = m_frame_buffer.head_index(),
                                .recovery_frames = current->p2_recovery_frames,
                                .game_frame = current->game_frame});
        if (m_logging) {
            log_info("MARK STARTUP P2: %i", current->game_frame);
        }
    }
}

ConnectionEvent FrameDataAnalyser::has_new_connection() {
    const game_state *const current = m_frame_buffer.head();
    const game_state *const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return ConnectionEvent::NO_CONNECTION;
    }

    if ((previous->p1_connection == 0) && (current->p1_connection != 0)) {
        return ConnectionEvent::P1_CONNECTION;
    } else if ((previous->p2_connection == 0) && (current->p2_connection != 0)) {
        return ConnectionEvent::P2_CONNECTION;
    }

    return ConnectionEvent::NO_CONNECTION;
}

StartFrame FrameDataAnalyser::get_startup_frame(const bool p2) {
    RingBuffer<StartFrame> *buffer = p2 ? &m_p2_start_frames : &m_p1_start_frames;

    const size_t item_count = buffer->item_count();
    for (size_t i = 0; i < item_count; i++) {
        const StartFrame start_frame = buffer->pop();
        // Reverse walk frames
        for (size_t j = 0; j < REVERSE_WALK_LIMIT; j++) {
            const game_state *frame = m_frame_buffer.get_from_head(j);
            const uint32_t last_action = p2 ? frame->p2_frames_last_action : frame->p1_frames_last_action;

            // Try to validate if the frame buffer has valid state
            if (frame->game_frame - start_frame.game_frame + 1 == last_action) {
                return start_frame;
            }
        }
    }

    log_fatal("player startup frame not found");
    return {};
}

void FrameDataAnalyser::handle_connection() {
    const ConnectionEvent connection = has_new_connection();
    if (connection == ConnectionEvent::NO_CONNECTION) {
        return;
    }

    const game_state *const current = m_frame_buffer.head();
    const game_state *const previous = m_frame_buffer.get_from_head(1);

    if (m_logging) {
        log_info("MARK CONNECTION P%i: %i", connection, current->game_frame);
    }

    StartFrame start{};
    uint32_t opponent_recovery_frames = 0;
    // Value in current frame
    uint32_t player_recovery_frames = 0;

    switch (connection) {
    case ConnectionEvent::NO_CONNECTION:
        return;
    case ConnectionEvent::P1_CONNECTION:
        m_p2_start_frames.clear();
        player_recovery_frames = current->p1_recovery_frames;
        opponent_recovery_frames = current->p2_recovery_frames;
        start = get_startup_frame(false);
        break;
    case ConnectionEvent::P2_CONNECTION:
        m_p1_start_frames.clear();
        player_recovery_frames = current->p2_recovery_frames;
        opponent_recovery_frames = current->p1_recovery_frames;
        start = get_startup_frame(true);
        break;
    }

    if (start.game_frame == 0) {
        log_error("Player start up buffer has invalid state");
        return;
    }

    int32_t startup_frames = 0;
    int32_t frame_advantage = 0;

    if (connection == ConnectionEvent::P1_CONNECTION) {
        startup_frames = (int) (current->game_frame - start.game_frame);
        // Don't base recovery time on startup frame if new recovery has begun
        if (p1_recovery_reset(previous, current)) {
            frame_advantage = (int) (opponent_recovery_frames - player_recovery_frames);
        } else {
            frame_advantage = (int) (startup_frames - (start.recovery_frames - opponent_recovery_frames));
        }
    } else {
        startup_frames = (int) (current->game_frame - start.game_frame);
        // Don't base recovery time on startup frame if new recovery has begun
        if (p2_recovery_reset(previous, current)) {
            frame_advantage = (int) (opponent_recovery_frames - player_recovery_frames);
        } else {
            frame_advantage = (int) ((start.recovery_frames - opponent_recovery_frames) - startup_frames);
        }
        startup_frames = 0;
    }

    const struct FrameDataPoint data_point = {.startup_frames = startup_frames, .frame_advantage = frame_advantage};

    m_listener->frame_data(data_point);
}

float FrameDataAnalyser::calculate_distance(const game_state *const state) {
    return (float) (std::sqrt(std::pow(state->p1_position.x - state->p2_position.x, 2) +
                              std::pow(state->p1_position.z - state->p2_position.z, 2)) /
                    1000);
}

void FrameDataAnalyser::handle_distance() {
    const game_state *const current = m_frame_buffer.head();
    const float distance = calculate_distance(current);
    m_listener->distance(distance);
}

void FrameDataAnalyser::handle_status() {
    const game_state *const current = m_frame_buffer.head();
    m_listener->status((PlayerState) current->p1_state);
}

bool FrameDataAnalyser::flip_player_data(game_state &state) {
    int32_t side_val = 0;
    const int result = player_side(&side_val);

    if (result == READ_ERROR) {
        log_error("failed to read player side");
        return false;
    }

    const auto side = (enum PlayerSide) side_val;
    const game_state temp_state = state;

    switch (side) {
    case PlayerSide::LEFT:
        break;

    case PlayerSide::RIGHT:
        state.p1_frames_last_action = temp_state.p2_frames_last_action;
        state.p1_recovery_frames = temp_state.p2_recovery_frames;
        state.p1_connection = temp_state.p2_connection;
        state.p1_intent = temp_state.p2_intent;
        state.p1_state = temp_state.p2_state;
        state.p1_position = temp_state.p2_position;

        state.p2_frames_last_action = temp_state.p1_frames_last_action;
        state.p2_recovery_frames = temp_state.p1_recovery_frames;
        state.p2_connection = temp_state.p1_connection;
        state.p2_intent = temp_state.p1_intent;
        state.p2_state = temp_state.p1_state;
        state.p2_position = temp_state.p1_position;
        break;

    default:
        log_error("unknown player side value %d", side);
        return false;
    }

    return true;
}

bool FrameDataAnalyser::update_game_state() {
    // Read the game's state
    game_state state{};
    if (read_game_state(&state) != READ_OK) {
        return false;
    }

    // Flip player data according to player side
    if (!flip_player_data(state)) {
        return false;
    }

    m_frame_buffer.push(state);

    return true;
}

bool FrameDataAnalyser::loop() {
    // Analyser timing
    uint32_t current_frame = 0;
    const int result = current_game_frame(&current_frame);
    if (result == READ_ERROR) {
        log_fatal("failed to read game's current frame number");
        return false;
    }

    const game_state *const previous = m_frame_buffer.head();

    // Check if the analyser is in sync with the game
    if (previous->game_frame != current_frame - 1 && previous->game_frame != 0) {
        const int64_t frames_off = (int64_t) previous->game_frame - (int64_t) current_frame;
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
    handle_status();

    if (m_logging) {
        log_frame();
    }

    return true;
}

bool FrameDataAnalyser::init(EventListener *listener) {
    if (listener == nullptr) {
        return false;
    }
    FrameDataAnalyser::m_listener = listener;

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

bool FrameDataAnalyser::start(EventListener *listener) {
    if (!init(listener)) {
        log_error("failed to init analyser");
        return false;
    }

    m_listener->game_hooked();

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
            std::this_thread::sleep_for(std::chrono::nanoseconds((TICK_LENGTH - nanos) / 2));

            end = std::chrono::high_resolution_clock::now();
            delta = end - start;
            nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count();
        }
    }

    return true;
}

void FrameDataAnalyser::stop() {
    m_stop = true;
}

bool FrameDataAnalyser::should_stop() {
    return m_stop;
}

void FrameDataAnalyser::set_logging(const bool enabled) {
    m_logging = enabled;
}
