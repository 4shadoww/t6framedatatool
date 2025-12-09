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
#include <cstddef>
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
#define PLAYER_ACTION_BUFFER_SIZE 10
#define PLAYER_STRING_BUFFER_SIZE 50
#define PLAYER_STRING_END_BUFFER_SIZE 3

//// frame_data_analyser
///
volatile bool FrameDataAnalyser::m_stop = false;
RingBuffer<GameFrame> FrameDataAnalyser::m_frame_buffer(FRAME_BUFFER_SIZE);
RingBuffer<StartFrame> FrameDataAnalyser::m_p1_start_frames(PLAYER_ACTION_BUFFER_SIZE);
RingBuffer<StartFrame> FrameDataAnalyser::m_p2_start_frames(PLAYER_ACTION_BUFFER_SIZE);
RingBuffer<GameFrame> FrameDataAnalyser::m_p1_str_connection_frames(PLAYER_STRING_BUFFER_SIZE);
RingBuffer<GameFrame> FrameDataAnalyser::m_p2_str_connection_frames(PLAYER_STRING_BUFFER_SIZE);
RingBuffer<GameFrame> FrameDataAnalyser::m_p1_str_end_frames(PLAYER_STRING_END_BUFFER_SIZE);
RingBuffer<GameFrame> FrameDataAnalyser::m_p2_str_end_frames(PLAYER_STRING_END_BUFFER_SIZE);

EventListener *FrameDataAnalyser::m_listener = nullptr;

// Avoid log spam
int FrameDataAnalyser::m_last_player_intent = 0;
bool FrameDataAnalyser::m_logging = false;

void FrameDataAnalyser::log_frame() {
    const GameFrame *const state = m_frame_buffer.head();
    std::stringstream stream;
    stream << "FRAME: " << state->game_frame << std::endl
           << "P1 last action: " << state->p1.frames_last_action << std::endl
           << "P1 recovery frames: " << state->p1.recovery_frames << std::endl
           << "P1 connection: " << (bool) state->p1.connection << std::endl
           << "P1 intent: " << state->p1.intent << std::endl
           << "P1 move: " << state->p1.move << std::endl
           << "P1 state: " << state->p1.state << std::endl
           << "P1 string type: " << state->p1.string_type << std::endl
           << "P1 string state: " << state->p1.string_state << std::endl
           << "P1 position: " << state->p1.position.x << ", " << state->p1.position.y << ", " << state->p1.position.z
           << std::endl
           << "P1 attack seq: " << state->p1.attack_seq << std::endl
           << "P2 last action: " << state->p2.frames_last_action << std::endl
           << "P2 recovery frames: " << state->p2.recovery_frames << std::endl
           << "P2 connection: " << (bool) state->p2.connection << std::endl
           << "P2 intent: " << state->p2.intent << std::endl
           << "P2 move: " << state->p2.move << std::endl
           << "P2 state: " << state->p2.state << std::endl
           << "P2 string type: " << state->p2.string_type << std::endl
           << "P2 string state: " << state->p2.string_state << std::endl
           << "P2 position: " << state->p2.position.x << ", " << state->p2.position.y << ", " << state->p2.position.z
           << std::endl
           << "P2 attack seq: " << state->p2.attack_seq;

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

bool FrameDataAnalyser::recovery_reset(const PlayerFrame *const previous, const PlayerFrame *const current) {
    return previous->move != current->move;
}

const GameFrame *FrameDataAnalyser::get_game_frame(const uint32_t game_frame) {
    const size_t count = m_frame_buffer.item_count();
    for (size_t i = 0; i < count; i++) {
        const GameFrame *const frame = m_frame_buffer.get_from_head(i);
        if (frame->game_frame == game_frame) {
            return frame;
        }
    }

    return nullptr;
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
    case PlayerState::STRING:
    case PlayerState::STANDING_HIT:
    case PlayerState::CROUCH_DASH_JUMP:
    case PlayerState::POWER_STANCE:
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

bool FrameDataAnalyser::initiated_attack(const PlayerFrame *const previous, const PlayerFrame *const current) {
    return ((PlayerMove) current->move != PlayerMove::IDLE && previous->move != current->move) ||
           (previous->attack_seq != current->attack_seq);
}

void FrameDataAnalyser::analyse_start_frames() {
    const GameFrame *const current = m_frame_buffer.head();
    const GameFrame *const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return;
    }

    // Check if P1 initiated attack
    if (initiated_attack(&previous->p1, &current->p1)) {
        m_p1_start_frames.push({.index = m_frame_buffer.head_index(),
                                .recovery_frames = current->p1.recovery_frames,
                                .game_frame = current->game_frame,
                                .attack_seq = current->p1.attack_seq});
        if (m_logging) {
            log_info("MARK STARTUP P1: %i", current->game_frame);
        }
    }

    // Check if P2 initiated attack
    if (initiated_attack(&previous->p2, &current->p2)) {
        m_p2_start_frames.push({.index = m_frame_buffer.head_index(),
                                .recovery_frames = current->p2.recovery_frames,
                                .game_frame = current->game_frame,
                                .attack_seq = current->p2.attack_seq});
        if (m_logging) {
            log_info("MARK STARTUP P2: %i", current->game_frame);
        }
    }
}

ConnectionEvent FrameDataAnalyser::has_new_connection() {
    const GameFrame *const current = m_frame_buffer.head();
    const GameFrame *const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return ConnectionEvent::NO_CONNECTION;
    }

    if ((previous->p1.connection == 0) && (current->p1.connection != 0)) {
        return ConnectionEvent::P1_CONNECTION;
    } else if ((previous->p2.connection == 0) && (current->p2.connection != 0)) {
        return ConnectionEvent::P2_CONNECTION;
    }

    return ConnectionEvent::NO_CONNECTION;
}

StartFrame FrameDataAnalyser::get_startup_frame(const GameFrame *const frame, const bool p2) {
    RingBuffer<StartFrame> *buffer = p2 ? &m_p2_start_frames : &m_p1_start_frames;
    const int32_t last_attack_seq = p2 ? frame->p2.attack_seq : frame->p1.attack_seq;

    const size_t item_count = buffer->item_count();
    for (size_t i = 0; i < item_count; i++) {
        const StartFrame start_frame = buffer->pop();

        if (start_frame.attack_seq == last_attack_seq) {
            return start_frame;
        }
        log_debug("dropped invalid startup frame %d", start_frame.game_frame);
    }

    log_fatal("player startup frame not found");
    return {};
}

bool FrameDataAnalyser::string_is_active(const PlayerFrame *player_frame) {
    return (PlayerState) player_frame->state == PlayerState::STRING ||
           (PlayerState) player_frame->state == PlayerState::MOVE;
}

bool FrameDataAnalyser::string_has_ended_state(const PlayerFrame *player_frame) {
    // NOTE: this function can only tell if the string has ended state, not if it's actually ongoing
    return (StringState) player_frame->string_state == StringState::ENDED;
}

bool FrameDataAnalyser::string_has_concluded(const GameFrame *current, const bool p2) {
    RingBuffer<GameFrame> *const str_end_frames = p2 ? &m_p2_str_end_frames : &m_p1_str_end_frames;

    str_end_frames->push(*current);

    // Collect more frames
    if (str_end_frames->item_count() < PLAYER_STRING_END_BUFFER_SIZE) {
        return false;
    }

    // Check consistency
    GameFrame *previous_check = str_end_frames->tail();
    for (size_t i = 1; i < PLAYER_STRING_END_BUFFER_SIZE - 1; i++) {
        GameFrame *const current_check = str_end_frames->get(i);
        // Inconsistent, continue collecting frames
        if (previous_check->game_frame + 1 != current_check->game_frame) {
            str_end_frames->pop();
            return false;
        }
        previous_check = current_check;
    }

    return true;
}

void FrameDataAnalyser::reset_string_sm() {
    // Clear connections
    m_p1_str_connection_frames.clear();
    m_p2_str_connection_frames.clear();
    // Clear end frames
    m_p1_str_end_frames.clear();
    m_p2_str_end_frames.clear();
}

bool FrameDataAnalyser::calculate_natural_string(RingBuffer<GameFrame> *const player_connections, const bool p2) {
    const GameFrame *const first_connection = player_connections->tail();
    const GameFrame *const first_previous_frame = get_game_frame(first_connection->game_frame - 1);
    const GameFrame *const last_connection = player_connections->head();
    const GameFrame *const last_previous_frame = get_game_frame(last_connection->game_frame - 1);

    if (first_previous_frame == nullptr) {
        log_error("cannot find previous frame for startup");
        reset_string_sm();
        return false;
    }

    if (last_previous_frame == nullptr) {
        log_error("cannot find previous frame for connection");
        reset_string_sm();
        return false;
    }

    const StartFrame first_startup = get_startup_frame(first_previous_frame, p2);
    if (first_startup.game_frame == 0) {
        log_error("Player start up buffer has invalid state (cannot get first)");
        reset_string_sm();
        return false;
    }

    // Check if first and last are the same
    StartFrame last_startup{};

    if (first_connection->game_frame == last_connection->game_frame) {
        last_startup = first_startup;
    } else {
        last_startup = get_startup_frame(last_previous_frame, p2);
        if (last_startup.game_frame == 0) {
            log_error("Player start up buffer has invalid state (cannot get last)");
            reset_string_sm();
            return false;
        }
    }

    log_debug("calculate natural string");
    // Calculate frame data for natural string
    int32_t startup_frames = (int) (first_connection->game_frame - first_startup.game_frame); // NOLINT
    const int last_startup_frames = (int) (last_connection->game_frame - last_startup.game_frame);
    int32_t frame_advantage = 0;

    if (p2) {
        // Don't base recovery time on startup frame if new recovery has begun
        if (recovery_reset(&last_previous_frame->p2, &last_connection->p2)) {
            frame_advantage = (int) (last_connection->p1.recovery_frames - last_connection->p2.recovery_frames);
        } else {
            frame_advantage =
                (int) ((last_startup.recovery_frames - last_connection->p1.recovery_frames) - last_startup_frames);
        }
        startup_frames = 0;
    } else {
        // Don't base recovery time on startup frame if new recovery has begun
        if (recovery_reset(&last_previous_frame->p1, &last_connection->p1)) {
            frame_advantage = (int) (last_connection->p2.recovery_frames - last_connection->p1.recovery_frames);
        } else {
            frame_advantage =
                (int) (last_startup_frames - (last_startup.recovery_frames - last_connection->p2.recovery_frames));
        }
    }

    const struct FrameDataPoint data_point = {.startup_frames = startup_frames, .frame_advantage = frame_advantage};

    m_listener->frame_data(data_point);

    reset_string_sm();

    return true;
}

bool FrameDataAnalyser::calculate_strings(const bool p2) {
    const GameFrame *const current = m_frame_buffer.head();
    const PlayerFrame *player{};
    RingBuffer<GameFrame> *player_connections{};

    if (p2) {
        player_connections = &m_p2_str_connection_frames;
        player = &current->p2;
    } else {
        player_connections = &m_p1_str_connection_frames;
        player = &current->p1;
    }

    // No string ended state, or no data: nothing to handle
    if (!string_has_ended_state(player) || player_connections->item_count() == 0) {
        return false;
    }

    // Check if the string has concluded (state may be ended for few frames, but string continues)
    if (!string_has_concluded(current, p2)) {
        return false;
    }

    // TODO: add multi-hit logic
    return calculate_natural_string(player_connections, p2);
}

void FrameDataAnalyser::calculate_single_attack(const ConnectionEvent connection,
                                                const GameFrame *const previous,
                                                const GameFrame *const current) {
    StartFrame startup{};
    // Value in current frame
    const PlayerFrame *player{};
    const PlayerFrame *opponent{};

    // Check frame before connection, as the player can initiate new attack on connection frame
    if (connection == ConnectionEvent::P1_CONNECTION) {
        m_p2_start_frames.clear();
        startup = get_startup_frame(previous, false);

        player = &current->p1;
        opponent = &current->p2;
    } else {
        m_p1_start_frames.clear();
        startup = get_startup_frame(previous, true);

        player = &current->p2;
        opponent = &current->p1;
    }

    if (startup.game_frame == 0) {
        log_error("Player start up buffer has invalid state");
        return;
    }

    log_debug("calculate single hit");

    // Calculate frame data for single attack
    int32_t startup_frames = (int) (current->game_frame - startup.game_frame); // NOLINT
    int32_t frame_advantage = 0;

    if (connection == ConnectionEvent::P1_CONNECTION) {
        // Don't base recovery time on startup frame if new recovery has begun
        if (recovery_reset(&previous->p1, player)) {
            frame_advantage = (int) (opponent->recovery_frames - player->recovery_frames);
        } else {
            frame_advantage = (int) (startup_frames - (startup.recovery_frames - opponent->recovery_frames));
        }
    } else {
        // Don't base recovery time on startup frame if new recovery has begun
        if (recovery_reset(&previous->p2, player)) {
            frame_advantage = (int) (opponent->recovery_frames - player->recovery_frames);
        } else {
            frame_advantage = (int) ((startup.recovery_frames - opponent->recovery_frames) - startup_frames);
        }
        startup_frames = 0;
    }

    const struct FrameDataPoint data_point = {.startup_frames = startup_frames, .frame_advantage = frame_advantage};

    m_listener->frame_data(data_point);
}

void FrameDataAnalyser::handle_connection() {
    const ConnectionEvent connection = has_new_connection();
    if (connection == ConnectionEvent::NO_CONNECTION) {
        return;
    }
    const GameFrame *const current = m_frame_buffer.head();
    const GameFrame *const previous = m_frame_buffer.get_from_head(1);

    if (m_logging) {
        log_info("MARK CONNECTION P%i: %i", connection, current->game_frame);
    }

    // Handle string later on separate function
    if (connection == ConnectionEvent::P1_CONNECTION) {
        if (string_is_active(&current->p1)) {
            m_p1_str_connection_frames.push(*current);
            return;
        }
    } else {
        if (string_is_active(&current->p2)) {
            m_p2_str_connection_frames.push(*current);
            return;
        }
    }

    calculate_single_attack(connection, previous, current);
}

void FrameDataAnalyser::handle_strings() {
    const GameFrame *const current = m_frame_buffer.head();

    // Try to caluculate P1
    if (string_is_active(&current->p1) && calculate_strings(false)) {
        return;
    }

    // Try to calculate P2
    if (string_is_active(&current->p2)) {
        calculate_strings(true);
    }
}

float FrameDataAnalyser::calculate_distance(const GameFrame *const state) {
    return (float) (std::sqrt(std::pow(state->p1.position.x - state->p2.position.x, 2) +
                              std::pow(state->p1.position.z - state->p2.position.z, 2)) /
                    1000);
}

void FrameDataAnalyser::handle_distance() {
    const GameFrame *const current = m_frame_buffer.head();
    const float distance = calculate_distance(current);
    m_listener->distance(distance);
}

void FrameDataAnalyser::handle_status() {
    const GameFrame *const current = m_frame_buffer.head();
    m_listener->status((PlayerState) current->p1.state);
}

bool FrameDataAnalyser::flip_player_data(GameFrame &state) {
    int32_t side_val = 0;
    const int result = player_side(&side_val);

    if (result == READ_ERROR) {
        log_error("failed to read player side");
        return false;
    }

    const auto side = (enum PlayerSide) side_val;
    const GameFrame temp_state = state;

    switch (side) {
    case PlayerSide::LEFT:
        break;

    case PlayerSide::RIGHT:
        state.p1 = temp_state.p2;
        state.p2 = temp_state.p1;
        break;

    default:
        log_error("unknown player side value %d", side);
        return false;
    }

    return true;
}

bool FrameDataAnalyser::update_game_state() {
    // Read the game's state
    GameFrame state{};
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

    const GameFrame *const previous = m_frame_buffer.head();

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

    analyse_start_frames();
    handle_connection();
    handle_strings();
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
