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

#include <stddef.h>

#include "moveset_structs.h"

_Static_assert(sizeof(struct Pushback) == 0x0C, "Pushback size");
_Static_assert(offsetof(struct Pushback, extradata_addr) == 0x08, "Pushback.extradata_addr");

_Static_assert(sizeof(struct HitCondition) == 0x0C, "HitCondition size");

_Static_assert(sizeof(struct Reactions) == 0x50, "Reactions size");
_Static_assert(offsetof(struct Reactions, front_counterhit_pushback) == 0x10, "Reactions.front_counterhit_pushback");
_Static_assert(offsetof(struct Reactions, block_pushback) == 0x18, "Reactions.block_pushback");

_Static_assert(sizeof(struct Cancel) == 0x20, "Cancel size");
_Static_assert(offsetof(struct Cancel, starting_frame) == 0x18, "Cancel.starting_frame");

_Static_assert(sizeof(struct Move) == 0x58, "Move size");
_Static_assert(offsetof(struct Move, cancel_addr) == 0x14, "Move.cancel_addr");
_Static_assert(offsetof(struct Move, move_id_val1) == 0x1C, "Move.moveId_val1");
_Static_assert(offsetof(struct Move, hit_condition_addr) == 0x20, "Move.hit_condition_addr");
_Static_assert(offsetof(struct Move, first_active_frame) == 0x4C, "Move.first_active_frame");
_Static_assert(offsetof(struct Move, last_active_frame) == 0x50, "Move.last_active_frame");

const struct Field pushback_fields[] = {
    F16(struct Pushback, duration, 1),
    F16(struct Pushback, displacement, 1),
    F32(struct Pushback, num_of_loops, 1),
    F32(struct Pushback, extradata_addr, 1),
};

const struct Field hit_condition_fields[] = {
    F32(struct HitCondition, requirements_addr, 1),
    F32(struct HitCondition, damage, 1),
    F32(struct HitCondition, reactions_addr, 1),
};

const struct Field reactions_fields[] = {
    F32(struct Reactions, front_pushback, 1),
    F32(struct Reactions, backturned_pushback, 1),
    F32(struct Reactions, left_side_pushback, 1),
    F32(struct Reactions, right_side_pushback, 1),
    F32(struct Reactions, front_counterhit_pushback, 1),
    F32(struct Reactions, downed_pushback, 1),
    F32(struct Reactions, block_pushback, 1),
    F16(struct Reactions, front_direction, 1),
    F16(struct Reactions, back_direction, 1),
    F16(struct Reactions, left_side_direction, 1),
    F16(struct Reactions, right_side_direction, 1),
    F16(struct Reactions, front_counterhit_direction, 1),
    F16(struct Reactions, downed_direction, 1),
    F32(struct Reactions, o_0x28_int, 1),
    F32(struct Reactions, o_0x2c_int, 1),
    F16(struct Reactions, vertical_pushback, 1),
    F16(struct Reactions, standing_moveid, 1),
    F16(struct Reactions, default_moveid, 1),
    F16(struct Reactions, crouch_moveid, 1),
    F16(struct Reactions, counterhit_moveid, 1),
    F16(struct Reactions, crouch_counterhit_moveid, 1),
    F16(struct Reactions, left_side_moveid, 1),
    F16(struct Reactions, crouch_left_side_moveid, 1),
    F16(struct Reactions, right_side_moveid, 1),
    F16(struct Reactions, crouch_right_side_moveid, 1),
    F16(struct Reactions, backturned_moveid, 1),
    F16(struct Reactions, crouch_backturned_moveid, 1),
    F16(struct Reactions, block_moveid, 1),
    F16(struct Reactions, crouch_block_moveid, 1),
    F16(struct Reactions, wallslump_moveid, 1),
    F16(struct Reactions, downed_moveid, 1),
};

/* direction and button are the two halves of command. The extractor swaps each half. */
const struct Field cancel_fields[] = {
    F32(struct Cancel, input.direction, 1),
    F32(struct Cancel, input.button, 1),
    F32(struct Cancel, requirements_addr, 1),
    F32(struct Cancel, extradata_addr, 1),
    F32(struct Cancel, detection_start, 1),
    F32(struct Cancel, detection_end, 1),
    F32(struct Cancel, starting_frame, 1),
    F16(struct Cancel, move_id, 1),
    F16(struct Cancel, cancel_option, 1),
};

const struct Field move_fields[] = {
    F32(struct Move, o_0x0_int, 0),
    F32(struct Move, o_0x4_int, 0),
    F32(struct Move, anim_addr, 1),
    F32(struct Move, vuln, 1),
    F32(struct Move, hitlevel, 1),
    F32(struct Move, cancel_addr, 1),
    F16(struct Move, transition, 1),
    F16(struct Move, move_id_val2, 1),
    F16(struct Move, move_id_val1, 1),
    F32(struct Move, hit_condition_addr, 1),
    F32(struct Move, anim_len, 1),
    F32(struct Move, airborne_start, 1),
    F32(struct Move, airborne_end, 1),
    F32(struct Move, ground_fall, 1),
    F32(struct Move, voicelip_addr, 1),
    F32(struct Move, extra_move_property_addr, 1),
    F32(struct Move, move_start_extraprop_addr, 1),
    F32(struct Move, move_end_extraprop_addr, 1),
    F32(struct Move, o_0x98_int, 1),
    F32(struct Move, hitbox_location, 0),
    F32(struct Move, first_active_frame, 1),
    F32(struct Move, last_active_frame, 1),
    F16(struct Move, o_0x6c_short, 1),
    F16(struct Move, distance, 1),
};

const size_t pushback_field_count = sizeof(pushback_fields) / sizeof(pushback_fields[0]);
const size_t hit_condition_field_count = sizeof(hit_condition_fields) / sizeof(hit_condition_fields[0]);
const size_t reactions_field_count = sizeof(reactions_fields) / sizeof(reactions_fields[0]);
const size_t cancel_field_count = sizeof(cancel_fields) / sizeof(cancel_fields[0]);
const size_t move_field_count = sizeof(move_fields) / sizeof(move_fields[0]);
