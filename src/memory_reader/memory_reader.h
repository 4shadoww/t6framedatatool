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

#ifndef MEMORY_READER_H
#define MEMORY_READER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// Memory reader init values
#define MR_INIT_OK 0
#define MR_INIT_ERROR -1

struct player_coordinate {
    float x;
    float y;
    float z;
};

struct game_state {
    uint32_t game_frame;

    int32_t p1_frames_last_action;
    uint32_t p1_recovery_frames;
    int8_t p1_connection;
    int32_t p1_intent;
    int32_t p1_state;
    struct player_coordinate p1_position;

    int32_t p2_frames_last_action;
    uint32_t p2_recovery_frames;
    int8_t p2_connection;
    int32_t p2_intent;
    int32_t p2_state;
    struct player_coordinate p2_position;
};

#define READ_ERROR -1
#define READ_OK 0

/**
* Finds T6 process ID and initializes the memory
*
* @return MR_INIT value
*/
int init_memory_reader(void);

int p1_frames_last_action(int32_t *value);
int p1_connection(int16_t *value);
int p1_recovery_frames(uint32_t *value);
int p1_intent(int32_t *value);
int p1_state(int32_t *value);
int p1_position(struct player_coordinate *value);

int p2_frames_last_action(int32_t *value);
int p2_connection(int16_t *value);
int p2_recovery_frames(uint32_t *value);
int p2_intent(int32_t *value);
int p2_state(int32_t *value);
int p2_position(struct player_coordinate *value);

int current_game_frame(uint32_t *value);
int player_side(int32_t *value);

/*
* Read game state to "state" struct
* @param pointer to state struct
* @return 0 on success, -1 on error
*/
int read_game_state(struct game_state *state);

#ifdef __cplusplus
};
#endif

#endif
