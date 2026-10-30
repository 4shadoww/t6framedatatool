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


#include "game_state_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "address_config.h"
#include "logging.h"
#include "memory_reader.h"
#include "memory_reader_types.h"
#include "moveset_structs.h"
#include "number_conversions.h"
#include "refl_struct.h"

struct Player {
    uint8_t player_num;
    uint64_t attack_seq_address;
    uint64_t moveset_info_address;
    struct MovesetInfo *moveset_info;
    struct Move *move;
    uint32_t move_count;
    struct HitCondition *hit_condition;
    uint32_t hit_condition_count;
    struct Requirement *requirement;
    uint32_t requirement_count;
    struct Reactions *reactions;
    uint32_t reactions_count;
    struct Cancel *cancel;
    uint32_t cancel_count;
    ps3address current_move;
};

static uint64_t g_player_side_address = 0;
static struct Player g_p1;
static struct Player g_p2;

static bool g_logging = false;

#define STRUCT_STRING_SIZE 3000

int read_refl_struct(const uint64_t address, // NOLINT(misc-use-internal-linkage)
                     void *out,
                     const size_t size,
                     const struct Field *fields,
                     const size_t field_count) {
    if (out == NULL || size == 0) {
        log_error("read_refl_struct called with no output");
        return READ_ERROR;
    }

    unsigned char raw[size];
    if (read_bytes_raw((long long) address, raw, size) < 0) {
        return READ_ERROR;
    }
    if (load_refl_struct(out, raw, size, fields, field_count) != REFL_OK) {
        return READ_ERROR;
    }
    return READ_OK;
}

static void allocate_memory(void) {
    // Not freed, same life time as whole program
    if (g_p1.moveset_info == NULL) {
        g_p1.moveset_info = malloc(sizeof(struct MovesetInfo));
    }

    if (g_p2.moveset_info == NULL) {
        g_p2.moveset_info = malloc(sizeof(struct MovesetInfo));
    }
}

static int read_player_moveset_info_mem(const uint64_t address, struct MovesetInfo *destination) {
    if (read_refl_struct((long long) address,
                         destination,
                         offsetof(struct MovesetInfo, table),
                         moveset_info_fields,
                         moveset_info_field_count) == READ_ERROR) {
        return READ_ERROR;
    }
    return read_refl_struct((long long) address + (long long) offsetof(struct MovesetInfo, table),
                            &destination->table,
                            sizeof(struct MovesetTable),
                            moveset_table_fields,
                            moveset_table_field_count);
}

static int load_table(void **const out,
                      uint32_t *const out_count,
                      const ps3address guest,
                      const uint32_t count,
                      const size_t stride,
                      const struct Field *fields,
                      const size_t field_count) {
    free(*out);
    *out = NULL;
    *out_count = 0;
    if (guest == 0 || count == 0) {
        return READ_OK;
    }

    const size_t table_size = count * stride;
    unsigned char *const data = malloc(table_size);
    if (data == NULL) {
        log_error("failed to allocate %d bytes", table_size);
        return READ_ERROR;
    }

    const uint64_t base = ps3_address_to_x64(guest);
    for (uint64_t i = 0; i < count; i++) {
        const uint64_t game_address = base + (i * stride);
        unsigned char *const output_address = data + (i * stride);

        if (read_refl_struct(game_address, output_address, stride, fields, field_count) == READ_ERROR) {
            free(data);
            return READ_ERROR;
        }
    }

    *out = data;
    *out_count = count;
    return READ_OK;
}

static int load_player_tables(struct Player *player) {
    const struct MovesetTable *table = &player->moveset_info->table;

    if (load_table((void **) &player->move,
                   &player->move_count,
                   table->move,
                   table->move_count,
                   sizeof(struct Move),
                   move_fields,
                   move_field_count) == READ_ERROR) {
        return READ_ERROR;
    }
    if (load_table((void **) &player->hit_condition,
                   &player->hit_condition_count,
                   table->hit_condition,
                   table->hit_condition_count,
                   sizeof(struct HitCondition),
                   hit_condition_fields,
                   hit_condition_field_count) == READ_ERROR) {
        return READ_ERROR;
    }
    if (load_table((void **) &player->requirement,
                   &player->requirement_count,
                   table->requirement,
                   table->requirement_count,
                   sizeof(struct Requirement),
                   requirement_fields,
                   requirement_field_count) == READ_ERROR) {
        return READ_ERROR;
    }
    if (load_table((void **) &player->reactions,
                   &player->reactions_count,
                   table->reactions,
                   table->reactions_count,
                   sizeof(struct Reactions),
                   reactions_fields,
                   reactions_field_count) == READ_ERROR) {
        return READ_ERROR;
    }
    return load_table((void **) &player->cancel,
                      &player->cancel_count,
                      table->cancel,
                      table->cancel_count,
                      sizeof(struct Cancel),
                      cancel_fields,
                      cancel_field_count);
}

static int read_player_moveset_info(long long pointer_address, struct Player *player, const uint32_t game_frame) {
    int32_t value = 0;
    uint64_t address = 0;

    if (read_4bytes(pointer_address, &value) == READ_ERROR) {
        return READ_ERROR;
    }
    if (value == 0) {
        return READ_OK;
    }
    address = ps3_address_to_x64(value);
    if (address == player->moveset_info_address) {
        return READ_OK;
    }

    log_info("reading new P%d moveset info for FRAME %d", player->player_num, game_frame);
    if (read_player_moveset_info_mem(address, player->moveset_info) == READ_ERROR) {
        log_error("failed to read P%d moveset info", player->player_num);
        return READ_ERROR;
    }
    if (load_player_tables(player) == READ_ERROR) {
        log_error("failed to read P%d moveset tables", player->player_num);
        return READ_ERROR;
    }
    player->moveset_info_address = address;

    if (g_logging) {
        char struct_string[STRUCT_STRING_SIZE];
        format_refl_struct(struct_string,
                           STRUCT_STRING_SIZE,
                           "moveset info:",
                           player->moveset_info,
                           moveset_info_fields,
                           moveset_info_field_count);
        log_info("P%d moveset info for FRAME %d:\n%s", player->player_num, game_frame, struct_string);

        format_refl_struct(struct_string,
                           STRUCT_STRING_SIZE,
                           "moveset table:",
                           &player->moveset_info->table,
                           moveset_table_fields,
                           moveset_table_field_count);
        log_info("P%d moveset table for FRAME %d:\n%s", player->player_num, game_frame, struct_string);
    }

    return READ_OK;
}

static int update_moveset_infos(const uint32_t game_frame) {
    if (read_player_moveset_info(P1_MOVESET_INFO_PTR, &g_p1, game_frame) == READ_ERROR) {
        log_error("failed to read P1 move table");
        return READ_ERROR;
    }

    if (read_player_moveset_info(P2_MOVESET_INFO_PTR, &g_p2, game_frame) == READ_ERROR) {
        log_error("failed to read P2 move table");
        return READ_ERROR;
    }
    return READ_OK;
}

int init_memory_reader(void) {
    allocate_memory();
    g_p1.player_num = 1;
    g_p2.player_num = 2;

    if (platform_init_memory_reader() == MR_INIT_ERROR) {
        return MR_INIT_ERROR;
    }
    // Player side
    int32_t value = 0;
    if (read_4bytes(PLAYER_SIDE_PTR, &value) == READ_ERROR) {
        return MR_INIT_ERROR;
    }

    value += PLAYER_SIDE_OFFSET;
    g_player_side_address = ps3_address_to_x64((ps3address) value);

    // Player 1 attack seq
    value = 0;
    if (read_4bytes(P1_ATTACK_SEQ_PTR, &value) == READ_ERROR) {
        return MR_INIT_ERROR;
    }

    value += P1_ATTACK_SEQ_OFFSET;
    g_p1.attack_seq_address = ps3_address_to_x64((ps3address) value);

    // Player 2 attack seq
    value = 0;
    if (read_4bytes(P2_ATTACK_SEQ_PTR, &value) == READ_ERROR) {
        return MR_INIT_ERROR;
    }

    value += P2_ATTACK_SEQ_OFFSET;
    g_p2.attack_seq_address = ps3_address_to_x64((ps3address) value);

    // Player moveset infos
    if (update_moveset_infos(0) == READ_ERROR) {
        return MR_INIT_ERROR;
    }

    return MR_INIT_OK;
}

static int guest_index(const ps3address guest, const ps3address base, const uint32_t count, const uint32_t stride) {
    if (stride == 0 || guest < base) {
        return -1;
    }
    const uint32_t offset = guest - base;
    if (offset % stride != 0 || offset / stride >= count) {
        return -1;
    }
    return (int) (offset / stride);
}

static struct Move *preloaded_move(const struct Player *player, const ps3address guest) {
    if (player->move == NULL) {
        return NULL;
    }
    const int index = guest_index(guest, player->moveset_info->table.move, player->move_count, sizeof(struct Move));
    if (index < 0) {
        return NULL;
    }
    return &player->move[index];
}

static struct Cancel *preloaded_cancel(const struct Player *player, const ps3address guest) {
    if (player->cancel == NULL) {
        return NULL;
    }
    const int index =
        guest_index(guest, player->moveset_info->table.cancel, player->cancel_count, sizeof(struct Cancel));
    if (index < 0) {
        return NULL;
    }
    return &player->cancel[index];
}

static const struct Cancel *idle_cancel(const struct Player *player, const ps3address guest) {
    const struct Cancel *cancel = preloaded_cancel(player, guest);
    if (cancel == NULL) {
        return NULL;
    }
    // The 0x8000 cancel ends the list and is taken automatically
    const struct Cancel *end = player->cancel + player->cancel_count;
    while (cancel < end) {
        if (cancel->input.direction == 0x8000) {
            return cancel;
        }
        cancel++;
    }
    return NULL;
}

#define ALIAS_COUNT 69

// Move ids >= 0x8000 are aliases; the alias array is still guest bytes
static int alias_move_index(const struct Player *owner, uint16_t move_id) {
    if (move_id < 0x8000) {
        return move_id;
    }
    const uint32_t alias = move_id - 0x8000U;
    if (alias >= ALIAS_COUNT) {
        return -1;
    }
    return (uint16_t) big16_to_little((const char *) &owner->moveset_info->current_aliases[alias]);
}

static const struct Cancel *reaction_idle_cancel(const struct Player *owner, uint16_t move_id) {
    const int index = alias_move_index(owner, move_id);
    if (index < 0 || (uint32_t) index >= owner->move_count || owner->move == NULL) {
        return NULL;
    }
    return idle_cancel(owner, owner->move[index].cancel_addr);
}

static void log_reaction_move(const struct Player *owner, const char *name, uint16_t move_id) {
    const struct Cancel *cancel = reaction_idle_cancel(owner, move_id);
    if (cancel == NULL) {
        log_info("    %s = %u (0x%04x) no idle cancel", name, move_id, move_id);
        return;
    }
    log_info("    %s = %u (0x%04x) move %d idle starting_frame %u",
             name,
             move_id,
             move_id,
             alias_move_index(owner, move_id),
             cancel->starting_frame);
}

static void log_reactions(const struct Player *owner, const struct Reactions *reactions) {
    char struct_string[STRUCT_STRING_SIZE];
    format_refl_struct(struct_string,
                       STRUCT_STRING_SIZE,
                       "reactions:",
                       reactions,
                       reactions_fields,
                       reactions_field_count);
    log_info("%s", struct_string);
    log_info("reaction moves:");
    log_reaction_move(owner, "standing_moveid", reactions->standing_moveid);
    log_reaction_move(owner, "crouch_moveid", reactions->crouch_moveid);
    log_reaction_move(owner, "counterhit_moveid", reactions->counterhit_moveid);
    log_reaction_move(owner, "crouch_counterhit_moveid", reactions->crouch_counterhit_moveid);
    log_reaction_move(owner, "block_moveid", reactions->block_moveid);
    log_reaction_move(owner, "crouch_block_moveid", reactions->crouch_block_moveid);
}

// Last requirement of a T6 requirement list
#define REQUIREMENT_END 397

static const struct Requirement *preloaded_requirement(const struct Player *owner, ps3address guest) {
    if (owner->requirement == NULL) {
        return NULL;
    }
    const int index = guest_index(guest,
                                  owner->moveset_info->table.requirement,
                                  owner->requirement_count,
                                  sizeof(struct Requirement));
    return index < 0 ? NULL : &owner->requirement[index];
}

static const struct Reactions *preloaded_reactions(const struct Player *owner, ps3address guest) {
    if (owner->reactions == NULL) {
        return NULL;
    }
    const int index =
        guest_index(guest, owner->moveset_info->table.reactions, owner->reactions_count, sizeof(struct Reactions));
    return index < 0 ? NULL : &owner->reactions[index];
}

static const struct HitCondition *preloaded_hit_condition(const struct Player *owner, ps3address guest) {
    if (owner->hit_condition == NULL) {
        return NULL;
    }
    const int index = guest_index(guest,
                                  owner->moveset_info->table.hit_condition,
                                  owner->hit_condition_count,
                                  sizeof(struct HitCondition));
    return index < 0 ? NULL : &owner->hit_condition[index];
}

// Resolves one hit condition row. Returns the row's first requirement condition in *condition.
static int read_hit_condition(const struct Player *fighter,
                              const struct Player *owner,
                              const struct HitCondition *hit_condition,
                              uint32_t row,
                              uint32_t *condition) {
    *condition = 0;
    if (hit_condition->requirements_addr != 0) {
        const struct Requirement *requirement = preloaded_requirement(owner, hit_condition->requirements_addr);
        if (requirement == NULL) {
            log_error("failed to read P%d requirement %u", fighter->player_num, row);
            return READ_ERROR;
        }
        *condition = requirement->condition;
    }

    if (hit_condition->reactions_addr == 0) {
        return READ_OK;
    }
    const struct Reactions *reactions = preloaded_reactions(owner, hit_condition->reactions_addr);
    if (reactions == NULL) {
        log_error("failed to read P%d reactions %u", fighter->player_num, row);
        return READ_ERROR;
    }

    if (g_logging) {
        log_info("P%d hit condition %u damage %u requirement %u",
                 fighter->player_num,
                 row,
                 hit_condition->damage,
                 *condition);
        log_reactions(owner, reactions);
    }
    return READ_OK;
}

// Walks the move's hit condition rows until the row whose requirement list is the end marker
static int
read_move_hit_conditions(const struct Player *fighter, const struct Player *owner, ps3address hit_condition_addr) {
    const struct HitCondition *hit_condition = preloaded_hit_condition(owner, hit_condition_addr);
    if (hit_condition == NULL) {
        log_error("failed to read P%d hit condition", fighter->player_num);
        return READ_ERROR;
    }
    const struct HitCondition *end = owner->hit_condition + owner->hit_condition_count;

    for (uint32_t row = 0; hit_condition < end; hit_condition++, row++) {
        uint32_t condition = 0;
        if (read_hit_condition(fighter, owner, hit_condition, row, &condition) == READ_ERROR) {
            return READ_ERROR;
        }
        if (condition == REQUIREMENT_END) {
            break;
        }
    }
    return READ_OK;
}

static struct Player *player_for_move(struct Player *player, ps3address guest) {
    if (preloaded_move(player, guest) != NULL) {
        return player;
    }
    struct Player *other = player == &g_p1 ? &g_p2 : &g_p1;
    if (preloaded_move(other, guest) != NULL) {
        return other;
    }
    return NULL;
}

static int read_move_structs(const uint64_t ptr_address,
                             struct PlayerFrame *player_frame,
                             struct Player *player,
                             const uint32_t game_frame) {
    //// Read move pointer
    int value = 0;

    if (read_4bytes((long long) ptr_address, &value) == READ_ERROR) {
        log_error("failed to read P%d move struct pointer", player->player_num);
        // Error fill with zeros
        player_frame->move.first_active_frame = 0;
        player_frame->move.last_active_frame = 0;
        return READ_ERROR;
    }

    // No active move, fill with zero
    if (value == 0) {
        player->current_move = 0;
        player_frame->move.first_active_frame = 0;
        player_frame->move.last_active_frame = 0;
        return READ_OK;
    }

    struct Player *owner = player_for_move(player, (ps3address) value);
    if (owner == NULL) {
        log_error("failed to read P%d move struct", player->player_num);
        return READ_ERROR;
    }
    const struct Move *move = preloaded_move(owner, (ps3address) value);

    // Active frames
    player_frame->move.first_active_frame = move->first_active_frame;
    player_frame->move.last_active_frame = move->last_active_frame;

    const struct Cancel *cancel = NULL;
    if (move->cancel_addr != 0) {
        cancel = idle_cancel(owner, move->cancel_addr);
        if (cancel == NULL) {
            const struct MovesetTable *table = &owner->moveset_info->table;
            log_error("failed to read P%d cancel 0x%08x (cancel table 0x%08x count %u, group cancel 0x%08x count %u)",
                      player->player_num,
                      move->cancel_addr,
                      table->cancel,
                      table->cancel_count,
                      table->group_cancel,
                      table->group_cancel_count);
        }
    }

    player_frame->move.cancel_startup = cancel == NULL ? 0 : cancel->starting_frame;

    if (g_logging) {
        char struct_string[STRUCT_STRING_SIZE];
        format_refl_struct(struct_string, STRUCT_STRING_SIZE, "move:", move, move_fields, move_field_count);
        log_info("P%d move structs for FRAME %d:\n%s", player->player_num, game_frame, struct_string);

        if (cancel != NULL) {
            format_refl_struct(struct_string, STRUCT_STRING_SIZE, "cancel:", cancel, cancel_fields, cancel_field_count);
            log_info("%s", struct_string);
        }
    }

    player->current_move = (ps3address) value;
    return read_move_hit_conditions(player, owner, move->hit_condition_addr);
}

int p1_frames_last_action(int32_t *value) {
    return read_4bytes(P1_FRAMES_LAST_ACTION, value);
}

int p1_connection(int16_t *value) {
    return read_2bytes(P1_CONNECTION_BOOL, value);
}

int p1_recovery_frames(uint32_t *value) {
    return read_4bytes(P1_RECOVERY_FRAMES, (int32_t *) value);
}

int p1_intent(int32_t *value) {
    return read_4bytes(P1_INTENT, value);
}

int p1_move_id(int32_t *value) {
    return read_4bytes(P1_MOVE, value);
}

int p1_state(int32_t *value) {
    return read_4bytes(P1_STATE, value);
}

int p1_string_type(int32_t *value) {
    return read_4bytes(P1_STRING_TYPE, value);
}

int p1_string_state(int32_t *value) {
    return read_4bytes(P1_STRING_STATE, value);
}

int p1_position(struct PlayerCoordinate *value) {
    int read = read_bytes_raw(P1_POSITION, value, sizeof(struct PlayerCoordinate));
    if (read == READ_ERROR) {
        return READ_ERROR;
    }

    value->x = big32_to_little_float((char *) value);
    value->y = big32_to_little_float(&((char *) value)[4]);
    value->z = big32_to_little_float(&((char *) value)[8]);

    return READ_OK;
}

int p1_attack_seq(int32_t *value) {
    return read_4bytes((long long) g_p1.attack_seq_address, value);
}

int p1_move_structs(struct PlayerFrame *player_frame, const uint32_t game_frame) {
    return read_move_structs(P1_MOVE_STRUCT_PTR, player_frame, &g_p1, game_frame);
}

int p2_frames_last_action(int32_t *value) {
    return read_4bytes(P2_FRAMES_LAST_ACTION, value);
}

int p2_connection(int16_t *value) {
    return read_2bytes(P2_CONNECTION_BOOL, value);
}

int p2_recovery_frames(uint32_t *value) {
    return read_4bytes(P2_RECOVERY_FRAMES, (int32_t *) value);
}

int p2_intent(int32_t *value) {
    return read_4bytes(P2_INTENT, value);
}

int p2_move_id(int32_t *value) {
    return read_4bytes(P2_MOVE, value);
}

int p2_state(int32_t *value) {
    return read_4bytes(P2_STATE, value);
}

int p2_string_type(int32_t *value) {
    return read_4bytes(P2_STRING_TYPE, value);
}

int p2_string_state(int32_t *value) {
    return read_4bytes(P2_STRING_STATE, value);
}

int p2_position(struct PlayerCoordinate *value) {
    int read = read_bytes_raw(P2_POSITION, value, sizeof(struct PlayerCoordinate));
    if (read == READ_ERROR) {
        return READ_ERROR;
    }

    value->x = big32_to_little_float((char *) value);
    value->y = big32_to_little_float(&((char *) value)[4]);
    value->z = big32_to_little_float(&((char *) value)[8]);

    return READ_OK;
}

int p2_attack_seq(int32_t *value) {
    return read_4bytes((long long) g_p2.attack_seq_address, value);
}

int p2_move_structs(struct PlayerFrame *player_frame, const uint32_t game_frame) {
    return read_move_structs(P2_MOVE_STRUCT_PTR, player_frame, &g_p2, game_frame);
}

int current_game_frame(uint32_t *value) {
    return read_4bytes(CURRENT_GAME_FRAME, (int32_t *) value);
}

int player_side(int32_t *value) {
    return read_4bytes((long long) g_player_side_address, value);
}

uint64_t player_side_address(void) {
    return g_player_side_address;
}

int read_game_state(struct GameFrame *state) {
    int32_t value = 0;
    struct PlayerCoordinate coords = {0, 0, 0};

    int result = current_game_frame((uint32_t *) &value);

    if (result == READ_ERROR) {
        log_debug("readed invalid game frame number");
        return READ_ERROR;
    }
    state->game_frame = (uint32_t) value;

    update_moveset_infos(value);

    result = p1_frames_last_action(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 last action frames");
        return READ_ERROR;
    }
    state->p1.frames_last_action = value;

    result = p1_connection((int16_t *) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 last connection frames");
        return READ_ERROR;
    }
    state->p1.connection = (int16_t) value; // NOLINT

    result = p1_recovery_frames((uint32_t *) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 recovery frames");
        return READ_ERROR;
    }
    state->p1.recovery_frames = (uint32_t) value;

    result = p1_intent(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 intent");
        return READ_ERROR;
    }
    state->p1.intent = value;

    result = p1_move_id(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 move");
        return READ_ERROR;
    }
    state->p1.move_id = value;

    result = p1_state(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p1.state = value;

    result = p1_string_type(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 string type");
        return READ_ERROR;
    }
    state->p1.string_type = value;

    result = p1_string_state(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 string state");
        return READ_ERROR;
    }
    state->p1.string_state = value;

    result = p1_position(&coords);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p1.position = coords;

    result = p1_attack_seq(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 attack seq");
        return READ_ERROR;
    }
    state->p1.attack_seq = value;

    result = p1_move_structs(&state->p1, state->game_frame);
    if (result == READ_ERROR) {
        log_debug("failed to read p1 move");
        return READ_ERROR;
    }

    result = p2_frames_last_action(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 last action frames");
        return READ_ERROR;
    }
    state->p2.frames_last_action = value;

    result = p2_connection((int16_t *) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 last connection frames");
        return READ_ERROR;
    }
    state->p2.connection = (int16_t) value; // NOLINT

    result = p2_recovery_frames((uint32_t *) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 recovery frames");
        return READ_ERROR;
    }
    state->p2.recovery_frames = value;

    result = p2_intent(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 intent");
        return READ_ERROR;
    }
    state->p2.intent = value;

    result = p2_move_id(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 move");
        return READ_ERROR;
    }
    state->p2.move_id = value;

    result = p2_state(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 state");
        return READ_ERROR;
    }
    state->p2.state = value;

    result = p2_string_type(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 string type");
        return READ_ERROR;
    }
    state->p2.string_type = value;

    result = p2_string_state(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 string state");
        return READ_ERROR;
    }
    state->p2.string_state = value;

    memset(&coords, 0, sizeof(coords)); // NOLINT

    result = p2_position(&coords);

    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p2.position = coords;

    result = p2_attack_seq(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 attack seq");
        return READ_ERROR;
    }
    state->p2.attack_seq = value;

    result = p2_move_structs(&state->p2, state->game_frame);
    if (result == READ_ERROR) {
        log_debug("failed to read p2 move");
        return READ_ERROR;
    }

    return 0;
}

void set_memory_reader_logging(const bool enabled) {
    g_logging = enabled;
}
