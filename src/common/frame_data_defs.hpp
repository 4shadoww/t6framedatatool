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

enum class PlayerSide : uint8_t {
    RIGHT = 0,
    LEFT = 1,
};

enum class HitLevel : uint8_t {
    NONE = 0,
    LOW = 1,
    MID = 2,
    SPECIAL_MID1 = 3,
    SPECIAL_MID2 = 4,
    HIGH = 5,
    UNBLOCKABLE_MID = 7,
    UNBLOCKABLE_HIGH = 8,
    THROW = 10,
};

enum class InputDirection : uint8_t {
    NEUTRAL = 0,
    UP = 1,
    DOWN = 2,
    BACK = 4,
    FORWARD = 8,
    UP_BACK = 5,
    UP_FORWARD = 9,
    DOWN_BACK = 6,
    DOWN_FORWARD = 10,
};

struct InputState {
    uint32_t game_frame;
    uint32_t raw;
    InputDirection direction;
    uint8_t buttons;
};

struct PlayerFrameData {
    int startup_frames;
    uint32_t first_active_frame;
    uint32_t last_active_frame;
    HitLevel hit_level;
    uint32_t damage;
    int frame_advantage;
    bool knock_down;
    bool is_attacker;
};

struct FrameDataPoint {
    PlayerFrameData p1;
    PlayerFrameData p2;
    // Local player side
    PlayerSide player_side;
    // Game frame this data was reported for
    uint32_t game_frame;
};

// Attack chain tracked from the moveset structs
struct AttackChain {
    // Game frame when the first attack of the chain started, zero when none
    uint32_t start_frame;
    // Game frame when the current animation started
    uint32_t anim_start;
    // Game frame of the first connection, zero until a hit lands
    uint32_t first_connection;
    uint32_t first_active_frame;
    uint32_t last_active_frame;
    HitLevel hit_level;
    uint32_t damage;
    // Game frame when the defender can act again, from the last connection
    uint32_t defender_free;
    bool knock_down;
    // Frame data has been emitted for this chain
    bool reported;
    // Last reported frame data, used for retroactive corrections
    FrameDataPoint reported_data;
    bool has_reported_data;
};

enum class PlayerState : int {
    UNKNOWN = 0,
    STANDING = 6482,
    STANDING_TRAINING = 6466,
    CROUCH = 538921,
    AOP_CROUCH = 788546,
    CROUCHING = 14633,
    CROUCHING_TRAINING = 14625,
    AOP_CROUCHING1 = 272417,
    AOP_CROUCHING2 = 796705,
    MOVE_BACKWARDS = 135250,
    MOVE_FORWARDS = 67650,
    RECOVER1 = 29698,
    RECOVER2 = 4178,
    STRING = 2114, // String, multi-hit, sidestep
    JUMPING = 51266,
    JUMPING_FORWARDS = 116802,
    JUMPING_BACKWARDS = 182338,
    AIRBORNE1 = 29702,
    AIRBORNE2 = 32262,
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
    DRAGONFLY = 53314,
    JIR_JUMP = 2148418,
    JIR_FLIP1 = 2716674, // flip on ground
    JIR_FLIP2 = 2623554, // flip on ground
    DES1_STANDING = 18474,
    DES2_STANDING = 16498,
    DES3_STANDING = 12329,
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

#endif
