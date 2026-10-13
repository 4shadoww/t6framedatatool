/*
  Copyright (C) 2026 Noa-Emil Nissinen

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

#ifndef FRAME_DATA_STRUCTS_HPP
#define FRAME_DATA_STRUCTS_HPP

#include <cstddef>
#include <cstdint>

struct FrameDataPoint {
    int startup_frames;
    uint32_t first_active_frame;
    uint32_t last_active_frame;
    int frame_advantage;
    bool knock_down;
};

struct StartFrame {
    size_t index;
    uint32_t recovery_frames;
    uint32_t game_frame;
    int32_t attack_seq;
    bool is_string;
    uint32_t first_active_frame;
    uint32_t last_active_frame;
};

enum class ConnectionEvent : uint8_t {
    NO_CONNECTION,
    P1_CONNECTION,
    P2_CONNECTION,
};

enum class PlayerState : int {
    UNKNOWN = 0,
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
    POWER_STANCE = 40960,
    SITTING = 50209,
    SPINNING1 = 8390722,
    SPINNING2 = 534561,
    SPIN_TO_STANDUP = 4196418,
    TRANSITION1 = 591938,
    TRANSITION2 = 641090,
    UNBLOCKABLE1 = 2107425,
    UNBLOCKABLE2 = 2099266,
    SLIDE1 = 16806918,
    SLIDE2 = 36996,
    STAND_SWORD = 2672706,
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
    LEFT = 1,
};

enum class PlayerMove : uint8_t {
    IDLE = 0,
};

enum class StringState : uint8_t {
    NOT_ENDED1 = 0,
    NOT_ENDED2 = 1, // Sometimes appears in midde of the string
    ENDED = 2,
};

enum class StringType : uint16_t {
    MULTIHIT0 = 1024,
    MULTIHIT1 = 1026,
    MULTIHIT2 = 1027,
};

#endif
