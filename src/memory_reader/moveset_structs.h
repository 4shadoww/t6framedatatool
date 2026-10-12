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

#ifndef MOVESET_STRUCTS_H
#define MOVESET_STRUCTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "refl_struct.h"

/* structs from TKMovesets github.com/Kiloutre/TKMovesets, credits: github.com/Kiloutre */

struct Pushback {
    uint16_t duration;
    uint16_t displacement;
    uint32_t num_of_loops;
    uint32_t extradata_addr;
};

struct HitCondition {
    uint32_t requirements_addr;
    uint32_t damage;
    uint32_t reactions_addr;
};

struct Reactions {
    uint32_t front_pushback;
    uint32_t backturned_pushback;
    uint32_t left_side_pushback;
    uint32_t right_side_pushback;
    uint32_t front_counterhit_pushback;
    uint32_t downed_pushback;
    uint32_t block_pushback;

    uint16_t front_direction;
    uint16_t back_direction;
    uint16_t left_side_direction;
    uint16_t right_side_direction;
    uint16_t front_counterhit_direction;
    uint16_t downed_direction;

    uint32_t o_0x28_int;
    uint32_t o_0x2c_int;

    uint16_t vertical_pushback;
    uint16_t standing_moveid;
    uint16_t default_moveid;
    uint16_t crouch_moveid;
    uint16_t counterhit_moveid;
    uint16_t crouch_counterhit_moveid;
    uint16_t left_side_moveid;
    uint16_t crouch_left_side_moveid;
    uint16_t right_side_moveid;
    uint16_t crouch_right_side_moveid;
    uint16_t backturned_moveid;
    uint16_t crouch_backturned_moveid;
    uint16_t block_moveid;
    uint16_t crouch_block_moveid;
    uint16_t wallslump_moveid;
    uint16_t downed_moveid;
};

struct CancelInput {
    uint32_t direction;
    uint32_t button;
};

struct Cancel {
    union {
        uint64_t command;
        struct CancelInput input;
    };
    uint32_t requirements_addr;
    uint32_t extradata_addr;
    uint32_t detection_start;
    uint32_t detection_end;
    uint32_t starting_frame;
    uint16_t move_id;
    uint16_t cancel_option;
};

struct Move {
    uint32_t o_0x0_int;
    uint32_t o_0x4_int;
    uint32_t anim_addr;
    uint32_t vuln;
    uint32_t hitlevel;
    uint32_t cancel_addr;
    uint16_t transition;
    uint16_t move_id_val2;
    uint16_t move_id_val1;
    uint32_t hit_condition_addr;
    int32_t anim_len;
    uint32_t airborne_start;
    uint32_t airborne_end;
    uint32_t ground_fall;
    uint32_t voicelip_addr;
    uint32_t extra_move_property_addr;
    uint32_t move_start_extraprop_addr;
    uint32_t move_end_extraprop_addr;
    int32_t o_0x98_int;
    uint32_t hitbox_location;
    uint32_t first_active_frame;
    uint32_t last_active_frame;
    int16_t o_0x6c_short;
    uint16_t distance;
};

extern const struct Field pushback_fields[];
extern const size_t pushback_field_count;

extern const struct Field hit_condition_fields[];
extern const size_t hit_condition_field_count;

extern const struct Field reactions_fields[];
extern const size_t reactions_field_count;

extern const struct Field cancel_fields[];
extern const size_t cancel_field_count;

extern const struct Field move_fields[];
extern const size_t move_field_count;

#ifdef __cplusplus
}
#endif

#endif
