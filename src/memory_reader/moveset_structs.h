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

#include "number_conversions.h"
#include "refl_struct.h"

/* structs from TKMovesets github.com/Kiloutre/TKMovesets, credits: github.com/Kiloutre */

struct Pushback {
    uint16_t duration;
    uint16_t displacement;
    uint32_t num_of_loops;
    ps3address extradata_addr;
};

struct HitCondition {
    ps3address requirements_addr;
    uint32_t damage;
    ps3address reactions_addr;
};

struct Reactions {
    ps3address front_pushback;
    ps3address backturned_pushback;
    ps3address left_side_pushback;
    ps3address right_side_pushback;
    ps3address front_counterhit_pushback;
    ps3address downed_pushback;
    ps3address block_pushback;

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
    ps3address requirements_addr;
    ps3address extradata_addr;
    uint32_t detection_start;
    uint32_t detection_end;
    uint32_t starting_frame;
    uint16_t move_id;
    uint16_t cancel_option;
};

struct Move {
    uint32_t o_0x0_int;
    uint32_t o_0x4_int;
    ps3address anim_addr;
    uint32_t vuln;
    uint32_t hitlevel;
    ps3address cancel_addr;
    uint16_t transition;
    uint16_t move_id_val2;
    uint16_t move_id_val1;
    ps3address hit_condition_addr;
    int32_t anim_len;
    uint32_t airborne_start;
    uint32_t airborne_end;
    uint32_t ground_fall;
    ps3address voicelip_addr;
    ps3address extra_move_property_addr;
    ps3address move_start_extraprop_addr;
    ps3address move_end_extraprop_addr;
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
