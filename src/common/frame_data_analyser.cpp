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
#include <thread>

#include "game_state_reader.h"
#include "logging.h"
#include "memory_reader_types.h"

#include "frame_data_analyser.hpp"

// Constants

// Run the tool twice as fast as the game to get accurate measurements
#define TICK_LENGTH 8333333

// Five seconds of frames
#define FRAME_BUFFER_SIZE (size_t) (60 * 5)
#define PLAYER_ACTION_BUFFER_SIZE 5
#define REVERSE_WALK_LIMIT 6

//// frame_data_analyser
///
volatile bool frame_data_analyser::m_stop = false;
ring_buffer<game_state> frame_data_analyser::m_frame_buffer(FRAME_BUFFER_SIZE);
ring_buffer<start_frame> frame_data_analyser::m_p1_start_frames(PLAYER_ACTION_BUFFER_SIZE);
ring_buffer<start_frame> frame_data_analyser::m_p2_start_frames(PLAYER_ACTION_BUFFER_SIZE);

event_listener *frame_data_analyser::m_listener = nullptr;

// Avoid log spam
int frame_data_analyser::last_player_intent = 0;

bool frame_data_analyser::is_attack(const player_intent intent) {
    switch (intent) {
    case player_intent::ATTACK1:
    case player_intent::ATTACK3:
    case player_intent::ATTACK5:
    case player_intent::ATTACK7:
        return true;

    case player_intent::IDLE:
    case player_intent::INPUT_BUFFERING:
    case player_intent::BLOCK:
    case player_intent::WALK:
    case player_intent::SIDE_STEP:
    case player_intent::DOUBLE_SIDE_STEP:
    case player_intent::FALLING:
    case player_intent::LANDING:
    case player_intent::STASIS:
    case player_intent::WHIFF:
    case player_intent::GRAP_INIT:
    case player_intent::GRAP_CONNECT:
    case player_intent::SIDE_ROLLING:
        return false;

    default:
        log_warn("unknown player intent \"%i\"", intent);
        return false;
    }
}

const char *frame_data_analyser::player_status(const player_state state) {
    static const char STANDING[] = "Standing";
    static const char CROUCH[] = "Crouch";
    static const char CROUCHING[] = "Crouching";
    static const char JUMPING[] = "Jumping";
    static const char AIRBORNE[] = "Airborne";
    static const char GROUNDED[] = "Grounded";
    static const char UNDETERMINABLE[] = "Undeterminable";

    switch (state) {
    case player_state::STANDING:
    case player_state::MOVE_BACKWARDS:
    case player_state::MOVE_FORWARDS:
    case player_state::DASH_BACKWARDS:
    case player_state::DASH_FORWARDS:
    case player_state::MOVE:
    case player_state::RECOVER1:
    case player_state::RECOVER2:
    case player_state::SIDE_STEPPING:
    case player_state::STANDING_HIT:
        return STANDING;
    case player_state::CROUCH:
        return CROUCH;
    case player_state::CROUCHING:
    case player_state::CROUCHING_ATTACK:
    case player_state::CROUCHING_BACKWARDS:
    case player_state::CROUCHING_FORWARDS:
        return CROUCHING;
    case player_state::JUMPING:
    case player_state::JUMPING_FORWARDS:
    case player_state::JUMPING_BACKWARDS:
        return JUMPING;
    case player_state::AIRBORNE:
        return AIRBORNE;
    case player_state::GROUNDED1:
    case player_state::GROUNDED2:
        return GROUNDED;
    }

    if (last_player_intent != (int) state) {
        log_debug("unknown player status %d", state);
        last_player_intent = (int) state;
    }

    return UNDETERMINABLE;
}

void frame_data_analyser::analyse_start_frames() {
    const game_state *const current = m_frame_buffer.head();
    const game_state *const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return;
    }

    // Check if P1 initiated attack
    if (is_attack((player_intent) current->p1_intent) && previous->p1_intent != current->p1_intent) {
        m_p1_start_frames.push({.index = m_frame_buffer.head_index(),
                                .recovery_frames = current->p1_recovery_frames,
                                .game_frame = current->game_frame});
    }

    // Check if P2 initiated attack
    if (is_attack((player_intent) current->p2_intent) && previous->p2_intent != current->p2_intent) {
        m_p2_start_frames.push({.index = m_frame_buffer.head_index(),
                                .recovery_frames = current->p2_recovery_frames,
                                .game_frame = current->game_frame});
    }
}

connection_event frame_data_analyser::has_new_connection() {
    const game_state *const current = m_frame_buffer.head();
    const game_state *const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return connection_event::NO_CONNECTION;
    }

    if ((previous->p1_connection == 0) && (current->p1_connection != 0)) {
        return connection_event::P1_CONNECTION;
    } else if ((previous->p2_connection == 0) && (current->p2_connection != 0)) {
        return connection_event::P2_CONNECTION;
    }

    return connection_event::NO_CONNECTION;
}

start_frame frame_data_analyser::get_startup_frame(const bool p2) {
    ring_buffer<start_frame> *buffer = p2 ? &m_p2_start_frames : &m_p1_start_frames;

    const size_t item_count = buffer->item_count();
    for (size_t i = 0; i < item_count; i++) {
        const start_frame start_frame = buffer->pop();
        // Reverse walk frames
        for (size_t j = 0; j < REVERSE_WALK_LIMIT; j++) {
            const game_state *frame = m_frame_buffer.get_from_head(j);
            const uint32_t last_action = p2 ? frame->p2_frames_last_action : frame->p1_frames_last_action;

            if (frame->game_frame - start_frame.game_frame + 1 == last_action) {
                return start_frame;
            }
        }
    }

    log_fatal("player startup frame not found");
    return {};
}


void frame_data_analyser::handle_connection() {
    const connection_event connection = has_new_connection();
    if (connection == connection_event::NO_CONNECTION) {
        return;
    }

    const game_state *const current = m_frame_buffer.head();

    start_frame start{};
    uint32_t opponent_recovery_frames = 0;

    switch (connection) {
    case connection_event::NO_CONNECTION:
        return;
    case connection_event::P1_CONNECTION:
        m_p2_start_frames.clear();
        opponent_recovery_frames = current->p2_recovery_frames;
        start = get_startup_frame(false);
        break;
    case connection_event::P2_CONNECTION:
        m_p1_start_frames.clear();
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

    if (connection == connection_event::P1_CONNECTION) {
        startup_frames = (int) (current->game_frame - start.game_frame);
        frame_advantage = (int) (startup_frames - (start.recovery_frames - opponent_recovery_frames));
    } else {
        startup_frames = (int) (current->game_frame - start.game_frame);
        frame_advantage = (int) ((start.recovery_frames - opponent_recovery_frames) - startup_frames);
        startup_frames = 0;
    }

    const struct frame_data_point data_point = {.startup_frames = startup_frames, .frame_advantage = frame_advantage};

    m_listener->frame_data(data_point);
}

float frame_data_analyser::calculate_distance(const game_state *const state) {
    return (float) (std::sqrt(std::pow(state->p1_position.x - state->p2_position.x, 2) +
                              std::pow(state->p1_position.z - state->p2_position.z, 2)) /
                    1000);
}

void frame_data_analyser::handle_distance() {
    const game_state *const current = m_frame_buffer.head();
    const float distance = calculate_distance(current);
    m_listener->distance(distance);
}

void frame_data_analyser::handle_status() {
    const game_state *const current = m_frame_buffer.head();
    m_listener->status((player_state) current->p1_state);
}

bool frame_data_analyser::flip_player_data(game_state &state) {
    int32_t side_val = 0;
    const int result = player_side(&side_val);

    if (result == READ_ERROR) {
        log_error("failed to read player side");
        return false;
    }

    const enum player_side side = (enum player_side)side_val;
    const game_state temp_state = state;

    switch (side) {
    case player_side::LEFT:
        break;

    case player_side::RIGHT:
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

bool frame_data_analyser::update_game_state() {
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

bool frame_data_analyser::loop() {
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

void frame_data_analyser::stop() {
    m_stop = true;
}
