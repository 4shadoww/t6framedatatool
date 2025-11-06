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

#include <string.h>

#include "address_config.h"
#include "logging.h"
#include "memory_reader.h"
#include "memory_reader_types.h"
#include "number_conversions.h"

static uint64_t g_player_side_address = 0;

int init_memory_reader(void) {
    if (platform_init_memory_reader() == MR_INIT_ERROR) {
        return MR_INIT_ERROR;
    }
    int32_t value = 0;
    if (read_4bytes(PLAYER_SIDE_PTR, &value) == READ_ERROR) {
        return MR_INIT_ERROR;
    }

    value += PLAYER_SIDE_OFFSET;
    g_player_side_address = ps3_address_to_x64((uint32_t) value);

    return MR_INIT_OK;
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

int32_t p1_intent(int32_t *value) {
    return read_4bytes(P1_INTENT, value);
}

int32_t p1_move(int32_t *value) {
    return read_4bytes(P1_MOVE, value);
}

int32_t p1_state(int32_t *value) {
    return read_4bytes(P1_STATE, value);
}

int p1_position(struct player_coordinate *value) {
    int read = read_bytes_raw(P1_POSITION, value, sizeof(struct player_coordinate));
    if (read == READ_ERROR) {
        return READ_ERROR;
    }

    value->x = big32_to_little_float((char *) value);
    value->y = big32_to_little_float(&((char *) value)[4]);
    value->z = big32_to_little_float(&((char *) value)[8]);

    return READ_OK;
}

int32_t p1_attack_seq(int32_t *value) {
    return read_4bytes(P1_ATTACK_SEQ, value);
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

int p2_move(int32_t *value) {
    return read_4bytes(P2_MOVE, value);
}

int p2_state(int32_t *value) {
    return read_4bytes(P2_STATE, value);
}

int p2_position(struct player_coordinate *value) {
    int read = read_bytes_raw(P2_POSITION, value, sizeof(struct player_coordinate));
    if (read == READ_ERROR) {
        return READ_ERROR;
    }

    value->x = big32_to_little_float((char *) value);
    value->y = big32_to_little_float(&((char *) value)[4]);
    value->z = big32_to_little_float(&((char *) value)[8]);

    return READ_OK;
}

int32_t p2_attack_seq(int32_t *value) {
    return read_4bytes(P2_ATTACK_SEQ, value);
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

int read_game_state(struct game_state *state) {
    int32_t value = 0;
    struct player_coordinate coords = {0, 0, 0};

    int result = current_game_frame((uint32_t *) &value);

    if (result == READ_ERROR) {
        log_debug("readed invalid game frame number");
        return READ_ERROR;
    }
    state->game_frame = (uint32_t) value;

    result = p1_frames_last_action(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 last action frames");
        return READ_ERROR;
    }
    state->p1_frames_last_action = value;

    result = p1_connection((int16_t *) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 last connection frames");
        return READ_ERROR;
    }
    state->p1_connection = (int16_t) value; // NOLINT

    result = p1_recovery_frames((uint32_t *) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 recovery frames");
        return READ_ERROR;
    }
    state->p1_recovery_frames = (uint32_t) value;

    result = p1_intent(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 intent");
        return READ_ERROR;
    }
    state->p1_intent = value;

    result = p1_move(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 move");
        return READ_ERROR;
    }
    state->p1_move = value;

    result = p1_state(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p1_state = value;

    result = p1_position(&coords);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p1_position = coords;

    result = p1_attack_seq(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 attack seq");
        return READ_ERROR;
    }
    state->p1_attack_seq = value;


    result = p2_frames_last_action(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 last action frames");
        return READ_ERROR;
    }
    state->p2_frames_last_action = value;

    result = p2_connection((int16_t *) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 last connection frames");
        return READ_ERROR;
    }
    state->p2_connection = (int16_t) value; // NOLINT

    result = p2_recovery_frames((uint32_t *) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 recovery frames");
        return READ_ERROR;
    }
    state->p2_recovery_frames = value;

    result = p2_intent(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 intent");
        return READ_ERROR;
    }
    state->p2_intent = value;

    result = p2_move(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 move");
        return READ_ERROR;
    }
    state->p2_move = value;

    result = p2_state(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 state");
        return READ_ERROR;
    }
    state->p2_state = value;

    memset(&coords, 0, sizeof(coords)); // NOLINT

    result = p2_position(&coords);

    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p2_position = coords;

    result = p2_attack_seq(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 attack seq");
        return READ_ERROR;
    }
    state->p2_attack_seq = value;

    return 0;
}
