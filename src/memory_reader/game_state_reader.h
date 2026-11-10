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

#ifndef GAME_STATE_READER_H
#define GAME_STATE_READER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

struct PlayerCoordinate {
    float x;
    float y;
    float z;
};

struct PlayerFrame {
    int32_t frames_last_action;
    uint32_t recovery_frames;
    int8_t connection;
    int32_t intent;
    int32_t move_id;
    int32_t state;
    struct PlayerCoordinate position;
    int32_t attack_seq;
    // Current move, kept until the next move becomes current
    struct {
        uint32_t first_active_frame;
        uint32_t last_active_frame;
        uint32_t cancel_startup;
        int32_t index;
        int32_t string_next;
        uint32_t hitlevel;
        uint32_t anim_addr;
        // Game frame when this move became current
        uint32_t start_frame;
    } move;
};

struct GameFrame {
    uint32_t game_frame;
    struct PlayerFrame p1;
    struct PlayerFrame p2;
};

struct RawPlayerInput {
    uint32_t bits;
};

int init_memory_reader(void);

int p1_frames_last_action(int32_t *value);
int p1_connection(int16_t *value);
int p1_recovery_frames(uint32_t *value);
int32_t p1_intent(int32_t *value);
int32_t p1_move_id(int32_t *value);
int32_t p1_state(int32_t *value);
int p1_position(struct PlayerCoordinate *value);
int32_t p1_attack_seq(int32_t *value);
int p1_move_structs(struct PlayerFrame *player_frame, const uint32_t game_frame);

int p2_frames_last_action(int32_t *value);
int p2_connection(int16_t *value);
int p2_recovery_frames(uint32_t *value);
int32_t p2_intent(int32_t *value);
int32_t p2_move_id(int32_t *value);
int32_t p2_state(int32_t *value);
int p2_position(struct PlayerCoordinate *value);
int32_t p2_attack_seq(int32_t *value);
int p2_move_structs(struct PlayerFrame *player_frame, const uint32_t game_frame);

int current_game_frame(uint32_t *value);
int player_side(int32_t *value);
uint64_t player_side_address(void);

/*
 * Read game state to "state" struct
 * @param pointer to state struct
 * @return 0 on success, -1 on error
 */
int read_game_state(struct GameFrame *state);

/*
 * Read player 1 raw input bits
 * @param input output raw input
 * @return 0 on success, -1 on error
 */
int read_raw_player_input(struct RawPlayerInput *input);

void set_memory_reader_logging(const bool enabled);

#ifdef __cplusplus
};
#endif

#endif
