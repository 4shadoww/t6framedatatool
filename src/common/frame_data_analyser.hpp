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

#ifndef FRAME_DATA_ANALYSER_HPP
#define FRAME_DATA_ANALYSER_HPP

#include "ring_buffer.hpp"

#include "game_state_reader.h"

struct FrameDataPoint {
    int startup_frames;
    int frame_advantage;
};

struct StartFrame {
    size_t index;
    uint32_t recovery_frames;
    uint32_t game_frame;
    int32_t attack_seq;
};

enum ConnectionEvent : uint8_t {
    NO_CONNECTION,
    P1_CONNECTION,
    P2_CONNECTION
};

enum class PlayerState : int {
    STANDING = 6482,
    CROUCH = 538921,
    CROUCHING = 14633,
    MOVE_BACKWARDS = 135250,
    MOVE_FORWARDS = 67650,
    RECOVER1 = 29698,
    RECOVER2 = 4178,
    STRING = 2114, // String, multi-hit, sidestep
    JUMPING = 51266,
    JUMPING_FORWARDS = 116802,
    JUMPING_BACKWARDS = 182338,
    AIRBORNE = 29702,
    GROUNDED1 = 24708,
    GROUNDED2 = 27268,
    DASH_FORWARDS = 1050690,
    DASH_BACKWARDS = 133186,
    MOVE = 526402,
    CROUCHING_ATTACK = 10273,
    CROUCHING_BACKWARDS = 143401,
    CROUCHING_FORWARDS = 75809,
    STANDING_HIT = 22562,
    CROUCH_DASH_JUMP = 575554,
};

enum class PlayerIntent : int {
    IDLE = 0,
    ATTACK1 = 1,
    ATTACK3 = 3,
    ATTACK5 = 5,
    ATTACK7 = 7, // Usually string
    INPUT_BUFFERING = 10,
    BLOCK = 11,
    WALK = 12,
    SIDE_STEP = 15,
    SIDE_ROLLING = 14,
    STASIS = 16,
    WHIFF = 17,
    DOUBLE_SIDE_STEP = 28,
    FALLING = 272, // While taking damage
    LANDING = 528, // While taking damage
    GRAP_INIT = 65539,
    GRAP_CONNECT = 65546,
};

enum class PlayerSide : uint8_t {
    RIGHT = 0,
    LEFT = 1
};

enum class PlayerMove : uint8_t {
    IDLE = 0
};

enum class StringState : uint8_t {
    NOT_ENDED1 = 0,
    NOT_ENDED2 = 1, // Sometimes appears in midde of the string
    ENDED = 2
};

class EventListener {
public:
    virtual ~EventListener() = default;
    EventListener() = default;
    EventListener(const EventListener &) = default;
    EventListener(EventListener &&) = default;
    EventListener &operator=(const EventListener &) = default;
    EventListener &operator=(EventListener &&) = default;

    virtual void frame_data(FrameDataPoint frame_data) = 0;
    virtual void distance(float distance) = 0;
    virtual void status(PlayerState status) = 0;
    virtual void game_hooked() = 0;
};

class FrameDataAnalyser {
public:
    FrameDataAnalyser() = delete;
    ~FrameDataAnalyser() = delete;

    FrameDataAnalyser(const FrameDataAnalyser &) = default;
    FrameDataAnalyser(FrameDataAnalyser &&) = delete;
    FrameDataAnalyser &operator=(const FrameDataAnalyser &) = default;
    FrameDataAnalyser &operator=(FrameDataAnalyser &&) = delete;

    /**
     * Hook-up to game's memory and start analysing frames
     *
     * @param callback callback pointer
     */
    static bool start(EventListener *listener);
    /**
     * Stop the analyser loop
     */
    static void stop();
    static bool should_stop();

    static const char *player_status(const PlayerState state);
    static void set_logging(const bool enabled);

private:
    static volatile bool m_stop;
    static RingBuffer<GameFrame> m_frame_buffer;
    static EventListener *m_listener;
    static int m_last_player_intent;
    static bool m_logging;

    // Analysis state
    static RingBuffer<StartFrame> m_p1_start_frames;
    static RingBuffer<StartFrame> m_p2_start_frames;
    static RingBuffer<StartFrame> m_p1_string_frames;
    static RingBuffer<StartFrame> m_p2_string_frames;


    static bool init(EventListener *listener);
    static bool loop();

    inline static void log_frame();
    inline static bool flip_player_data(GameFrame &state);
    inline static bool is_attack(const PlayerIntent &intent);
    inline static bool recovery_reset(const PlayerFrame *const previous, const PlayerFrame *const current);
    static StartFrame get_startup_frame(const bool p2);

    inline static bool initiated_attack(const PlayerFrame *const previous, const PlayerFrame *const current);
    static void analyse_start_frames();
    static bool update_game_state();
    static ConnectionEvent has_new_connection();
    inline static bool string_is_active(const PlayerFrame *const player_frame);
    inline static bool string_has_ended(const PlayerFrame *const player_frame);
    static void handle_connection();
    static void handle_distance();
    static void handle_status();

    static float calculate_distance(const GameFrame *const state);
};

#endif
