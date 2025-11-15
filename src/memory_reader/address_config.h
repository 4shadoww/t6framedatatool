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

#ifndef ADDRESS_CONFIG_H
#define ADDRESS_CONFIG_H

// Player 1 health
#define GAME_BASE_ADDRESS 0x300B2C140
// Player 1 frames since last action
#define P1_FRAMES_LAST_ACTION GAME_BASE_ADDRESS + 124
// Plater 1 animation recovery frames
#define P1_RECOVERY_FRAMES GAME_BASE_ADDRESS + 392
// Plater 1 connection boolean
#define P1_CONNECTION_BOOL GAME_BASE_ADDRESS + 915132194
// Player 1 intent
#define P1_INTENT GAME_BASE_ADDRESS + 472
// Player 1 move
#define P1_MOVE GAME_BASE_ADDRESS + 332
// Player 1 state
#define P1_STATE GAME_BASE_ADDRESS + 172
// Player 1 coordinates
#define P1_POSITION GAME_BASE_ADDRESS + 2192
// Player 2 attack sequence number pointer
#define P1_ATTACK_SEQ_PTR GAME_BASE_ADDRESS + 342632
// Offset from pointer to attack seq value
#define P1_ATTACK_SEQ_OFFSET 312

// Player 2 frames since last action
#define P2_FRAMES_LAST_ACTION GAME_BASE_ADDRESS + 3260
// Plater 2 animation recovery frames
#define P2_RECOVERY_FRAMES GAME_BASE_ADDRESS + 3528
// Plater 2 connection boolean
#define P2_CONNECTION_BOOL GAME_BASE_ADDRESS + 915132186
// Player 2 intent
#define P2_INTENT GAME_BASE_ADDRESS + 3608
// Player 2 move
#define P2_MOVE GAME_BASE_ADDRESS + 3468
// Player 1 state
#define P2_STATE GAME_BASE_ADDRESS + 3308
// Player 2 coordinates
#define P2_POSITION GAME_BASE_ADDRESS + 5248
// Player 2 attack sequence number pointer
#define P2_ATTACK_SEQ_PTR GAME_BASE_ADDRESS + 342636
// Offset from pointer to attack seq value
#define P2_ATTACK_SEQ_OFFSET 312

// Current game frame
#define CURRENT_GAME_FRAME GAME_BASE_ADDRESS + 270587472

// Player side ptr
#define PLAYER_SIDE_PTR GAME_BASE_ADDRESS + 915193080
#define PLAYER_SIDE_OFFSET 21808


// Dynamic data section base pointer
// #define GAME_DYN_DATA_PTR = 0x300C3F9E8
// Player 1 frames since last connection
// #define P1_FRAMES_LAST_CONNECTION_PTR_OFFSET 913922468

#endif
