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

#include "refl_struct.h"
#include "ring_buffer.hpp"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <thread>

#include "frame_dumper.h"
#include "game_state_reader.h"
#include "logging.h"
#include "memory_reader_types.h"

#include "frame_data_analyser.hpp"

// Constants

// Run the tool twice as fast as the game to get accurate measurements
#define TICK_LENGTH 8333333

// Ten seconds of frames
#define FRAME_BUFFER_SIZE (size_t) (60 * 10)

//// frame_data_analyser
///
volatile bool FrameDataAnalyser::m_stop = false;
RingBuffer<GameFrame> FrameDataAnalyser::m_frame_buffer(FRAME_BUFFER_SIZE);
AttackChain FrameDataAnalyser::m_p1_chain{};
AttackChain FrameDataAnalyser::m_p2_chain{};

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
           << "P1 move: " << state->p1.move_id << std::endl
           << "P1 state: " << state->p1.state << std::endl
           << "P1 position: " << state->p1.position.x << ", " << state->p1.position.y << ", " << state->p1.position.z
           << std::endl
           << "P1 attack seq: " << state->p1.attack_seq << std::endl
           << "P1 move index: " << state->p1.move.index << std::endl
           << "P1 move start: " << state->p1.move.start_frame << std::endl
           << "P1 idle starting_frame: " << state->p1.move.cancel_startup << std::endl
           << "P1 string next: " << state->p1.move.string_next << std::endl
           << "P1 chain start: " << m_p1_chain.start_frame << std::endl
           << "P1 anim start: " << m_p1_chain.anim_start << std::endl
           << "P2 last action: " << state->p2.frames_last_action << std::endl
           << "P2 recovery frames: " << state->p2.recovery_frames << std::endl
           << "P2 connection: " << (bool) state->p2.connection << std::endl
           << "P2 intent: " << state->p2.intent << std::endl
           << "P2 move: " << state->p2.move_id << std::endl
           << "P2 state: " << state->p2.state << std::endl
           << "P2 position: " << state->p2.position.x << ", " << state->p2.position.y << ", " << state->p2.position.z
           << std::endl
           << "P2 attack seq: " << state->p2.attack_seq << std::endl
           << "P2 move index: " << state->p2.move.index << std::endl
           << "P2 move start: " << state->p2.move.start_frame << std::endl
           << "P2 idle starting_frame: " << state->p2.move.cancel_startup << std::endl
           << "P2 string next: " << state->p2.move.string_next << std::endl
           << "P2 chain start: " << m_p2_chain.start_frame << std::endl
           << "P2 anim start: " << m_p2_chain.anim_start << std::endl;

    log_info(stream.str().c_str());
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
    case PlayerState::SPINNING1:
    case PlayerState::SPINNING2:
    case PlayerState::SPIN_TO_STANDUP:
    case PlayerState::TRANSITION1:
    case PlayerState::TRANSITION2:
    case PlayerState::UNBLOCKABLE1:
    case PlayerState::UNBLOCKABLE2:
    case PlayerState::SLIDE1:
    case PlayerState::SLIDE2:
    case PlayerState::STAND_SWORD:
        return STANDING;
    case PlayerState::CROUCH:
    case PlayerState::SITTING:
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
    case PlayerState::AIRBORNE1:
    case PlayerState::AIRBORNE2:
        return AIRBORNE;
    case PlayerState::GROUNDED1:
    case PlayerState::GROUNDED2:
        return GROUNDED;
    case PlayerState::UNKNOWN:
        break;
    }

    if (m_last_player_intent != (int) state) {
        log_debug("unknown player status %d", state);
        m_last_player_intent = (int) state;
    }

    return UNDETERMINABLE;
}

// A reaction move without an idle return does not recover on its own
bool FrameDataAnalyser::is_knockdown(const PlayerFrame *const player_frame) {
    if (player_frame->move.cancel_startup == 0) {
        return true;
    }

    switch ((PlayerState) player_frame->state) {
    case PlayerState::AIRBORNE1:
    case PlayerState::AIRBORNE2:
    case PlayerState::GROUNDED1:
    case PlayerState::GROUNDED2:
        return true;

    default:
        return false;
    }
}

// Updates the chain when a move became current on this frame.
// A move is chained when it was entered from an attack that had not reached
// its idle cancel yet, or through an automatic string cancel.
void FrameDataAnalyser::track_chain(const PlayerFrame *const previous,
                                    const PlayerFrame *const current,
                                    AttackChain *const chain,
                                    const uint32_t game_frame) {
    if (current->move.index < 0 || current->move.start_frame != game_frame) {
        return;
    }

    const bool previous_attack = previous->move.index >= 0 && previous->move.hitlevel != 0;
    const bool auto_string = previous_attack && previous->move.string_next == current->move.index;
    const bool cancelled = previous_attack && (game_frame - previous->move.start_frame) < previous->move.cancel_startup;
    const bool chained = chain->start_frame != 0 && (auto_string || cancelled);

    report_replaced_string(previous, current, chain, auto_string);

    if (!chained || left_before_hitbox(previous, current, chain, auto_string, game_frame)) {
        if (current->move.hitlevel == 0) {
            *chain = {};
            return;
        }
        *chain = {};
        chain->start_frame = game_frame;
        chain->first_active_frame = current->move.first_active_frame;
        chain->last_active_frame = current->move.last_active_frame;
        if (m_logging) {
            log_info("MARK STARTUP P%d: %u", chain == &m_p1_chain ? 1 : 2, game_frame);
        }
    } else if (chain->first_connection == 0 && chain->last_active_frame == 0) {
        // The chain started from a move without a hitbox, the active window comes from this move
        chain->first_active_frame = current->move.first_active_frame;
        chain->last_active_frame = current->move.last_active_frame;
    }

    // The animation frame continues only through an automatic string in the same animation
    if (!auto_string || previous->move.anim_addr != current->move.anim_addr) {
        chain->anim_start = game_frame;
    }
}

void FrameDataAnalyser::track_chains() {
    const GameFrame *const current = m_frame_buffer.head();
    const GameFrame *const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return;
    }

    track_chain(&previous->p1, &current->p1, &m_p1_chain, current->game_frame);
    track_chain(&previous->p2, &current->p2, &m_p2_chain, current->game_frame);
}

bool FrameDataAnalyser::new_connection(const PlayerFrame *const previous, const PlayerFrame *const current) {
    return previous->connection == 0 && current->connection != 0;
}

// The previous hitbox never came out and the follow-up is another attack
bool FrameDataAnalyser::left_before_hitbox(const PlayerFrame *const previous,
                                           const PlayerFrame *const current,
                                           const AttackChain *const chain,
                                           const bool auto_string,
                                           const uint32_t game_frame) {
    return previous->move.index >= 0 && previous->move.hitlevel != 0 && chain->first_connection == 0 && !auto_string &&
           current->move.hitlevel != 0 && previous->move.first_active_frame != 0 &&
           previous->move.anim_addr != current->move.anim_addr &&
           (game_frame - previous->move.start_frame) < previous->move.first_active_frame;
}

// The automatic string did not continue, so the recorded hit is the finisher
void FrameDataAnalyser::report_replaced_string(const PlayerFrame *const previous,
                                               const PlayerFrame *const current,
                                               AttackChain *const chain,
                                               const bool auto_string) {
    if (chain->first_connection == 0 || chain->reported || auto_string) {
        return;
    }

    uint32_t recovery = previous->move.cancel_startup;
    if (current->move.hitlevel == 0 && current->move.anim_addr == previous->move.anim_addr &&
        current->move.cancel_startup != 0) {
        recovery = current->move.cancel_startup;
    }
    report_chain(chain, chain == &m_p2_chain, recovery);
}

// Frame advantage from the idle cancel of the finisher and the recovery of the
// defender recorded on the last connection
void FrameDataAnalyser::report_chain(AttackChain *const chain, const bool p2, const uint32_t recovery) {
    const uint32_t attacker_free = chain->anim_start + recovery;

    int32_t frame_advantage = (int32_t) chain->defender_free - (int32_t) attacker_free;
    auto startup_frames = (int32_t) (chain->first_connection - chain->start_frame);
    uint32_t first_active_frame = chain->first_active_frame;
    uint32_t last_active_frame = chain->last_active_frame;

    // Frame data is shown from P1's point of view
    if (p2) {
        frame_advantage = -frame_advantage;
        startup_frames = 0;
        first_active_frame = 0;
        last_active_frame = 0;
    }

    const struct FrameDataPoint data_point = {
        .startup_frames = startup_frames,
        .first_active_frame = first_active_frame,
        .last_active_frame = last_active_frame,
        .frame_advantage = frame_advantage,
        .knock_down = chain->knock_down,
    };

    chain->reported = true;
    m_listener->frame_data(data_point);
}

// Records the defender's recovery on a hit. Called before the chain is updated
// for this frame, so the chain still describes the move that was active when
// the hit landed.
void FrameDataAnalyser::calculate_connection(const GameFrame *const previous,
                                             const GameFrame *const current,
                                             const bool p2) {
    const PlayerFrame *attacker = p2 ? &current->p2 : &current->p1;
    const PlayerFrame *const defender = p2 ? &current->p1 : &current->p2;
    AttackChain *const chain = p2 ? &m_p2_chain : &m_p1_chain;
    const uint32_t frame = current->game_frame;

    // A cancel fired on the hit frame, the connecting move is the previous one
    if (attacker->move.start_frame == frame) {
        attacker = p2 ? &previous->p2 : &previous->p1;
    }

    if (m_logging) {
        log_info("MARK CONNECTION P%d: %u", p2 ? 2 : 1, frame);
    }

    if (chain->start_frame == 0 || attacker->move.index < 0) {
        log_debug("connection without tracked attack");
        return;
    }

    if (defender->move.index < 0) {
        log_debug("connection without defender move");
        return;
    }

    if (chain->first_connection == 0) {
        chain->first_connection = frame;
    }
    chain->defender_free = defender->move.start_frame + defender->move.cancel_startup;
    chain->knock_down = is_knockdown(defender);
    chain->reported = false;

    // More hits of an automatic string are scheduled, wait for the finisher
    if (attacker->move.string_next >= 0) {
        return;
    }

    report_chain(chain, p2, attacker->move.cancel_startup);
}

// Emits the frame data when the finisher's active window passed without a hit
void FrameDataAnalyser::handle_whiff(const PlayerFrame *const attacker, AttackChain *const chain, const bool p2) {
    if (chain->start_frame == 0 || chain->first_connection == 0 || chain->reported || attacker->move.index < 0 ||
        attacker->move.string_next >= 0) {
        return;
    }

    const uint32_t frame = m_frame_buffer.head()->game_frame;
    if (frame - chain->anim_start <= attacker->move.last_active_frame) {
        return;
    }

    if (m_logging) {
        log_info("MARK WHIFF P%d: %u", p2 ? 2 : 1, frame);
    }
    report_chain(chain, p2, attacker->move.cancel_startup);
}

void FrameDataAnalyser::handle_whiffs() {
    const GameFrame *const current = m_frame_buffer.head();
    handle_whiff(&current->p1, &m_p1_chain, false);
    handle_whiff(&current->p2, &m_p2_chain, true);
}

void FrameDataAnalyser::handle_connection() {
    const GameFrame *const current = m_frame_buffer.head();
    const GameFrame *const previous = m_frame_buffer.get_from_head(1);

    if (current == previous || previous == nullptr) {
        return;
    }

    if (new_connection(&previous->p1, &current->p1)) {
        calculate_connection(previous, current, false);
    }
    if (new_connection(&previous->p2, &current->p2)) {
        calculate_connection(previous, current, true);
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

bool FrameDataAnalyser::flip_player_data(GameFrame &state, const int32_t side_val) {
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

    int32_t side_val = 0;
    if (player_side(&side_val) == READ_ERROR) {
        log_error("failed to read player side");
        return false;
    }

    // Record the frame before flipping so a replay can repeat the flip
    if (frame_dump_write(&state, side_val) != READ_OK) {
        return false;
    }

    return process_frame(state, side_val);
}

bool FrameDataAnalyser::process_frame(GameFrame state, const int32_t side_val) {
    if (!flip_player_data(state, side_val)) {
        return false;
    }

    return process_frame(state);
}

bool FrameDataAnalyser::process_frame(GameFrame state) {
    m_frame_buffer.push(state);

    handle_connection();
    track_chains();
    handle_whiffs();
    handle_distance();
    handle_status();

    if (m_logging) {
        log_frame();
    }

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
        log_fatal("failed to update game's state");
        return false;
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
        log_fatal("failed to update game's state");
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
        const auto start = std::chrono::high_resolution_clock::now();

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

void FrameDataAnalyser::reset(EventListener *listener) {
    m_stop = false;
    m_frame_buffer.clear();
    m_listener = listener;
    m_last_player_intent = 0;
    m_logging = false;
    m_p1_chain = {};
    m_p2_chain = {};
}

void FrameDataAnalyser::set_logging(const bool enabled) {
    m_logging = enabled;
}
