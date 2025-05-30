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

#include "memory_reader.h"

struct frame_data_point {
    int startup_frames;
    int frame_advantage;
};

struct start_frame {
    size_t index;
    uint32_t recovery_frames;
    uint32_t game_frame;
};

enum connection_event {
    NO_CONNECTION,
    P1_CONNECTION,
    P2_CONNECTION
};

enum class player_state : int {
    STANDING = 6482,
    CROUCH = 538921,
    CROUCHING = 14633,
    MOVE_BACKWARDS = 135250,
    MOVE_FORWARDS = 67650,
    RECOVER1 = 29698,
    RECOVER2 = 4178,
    SIDE_STEPPING = 2114,
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
};

enum class player_intent : int {
    IDLE = 0,
    ATTACK1 = 1,
    ATTACK3 = 3,
    ATTACK5 = 5,
    ATTACK7 = 7, // Usually string
    INPUT_BUFFERING = 10,
    BLOCK = 11,
    WALK = 12,
    SIDE_STEP = 15,
    STASIS = 16,
    WHIFF = 17,
    DOUBLE_SIDE_STEP = 28,
    FALLING = 272, // While taking damage
    LANDING = 528, // While taking damage
    GRAP_INIT = 65539,
    GRAP_CONNECT = 65546,
};

enum class player_side : int {
    RIGHT = 0,
    LEFT = 1
};

class event_listener {
public:
    virtual void frame_data(frame_data_point frame_data) = 0;
    virtual void distance(float distance) = 0;
    virtual void status(player_state status) = 0;
    virtual void game_hooked() = 0;
};

class frame_data_analyser {
public:
    frame_data_analyser() = delete;
    ~frame_data_analyser() = delete;

    /**
     * Hook-up to game's memory and start analysing frames
     *
     * @param callback callback pointer
     */
    static bool start(event_listener *listener);
    /**
     * Stop the analyser loop
     */
    static void stop();

    static const char *player_status(const player_state state);
private:
    static bool m_stop;
    static ring_buffer<game_state> m_frame_buffer;
    static event_listener *m_listener;

    // Analysis state
    static ring_buffer<start_frame> m_p1_start_frames;
    static ring_buffer<start_frame> m_p2_start_frames;


    static bool init(event_listener *listener);
    static bool loop();

    inline static bool flip_player_data(game_state &state);
    inline static bool is_attack(const player_intent action);
    static start_frame get_startup_frame(const bool p2);

    static void analyse_start_frames();
    static bool update_game_state();
    static connection_event has_new_connection();
    static void handle_connection();
    static void handle_distance();
    static void handle_status();

    static float calculate_distance(const game_state * const state);
};

#endif
