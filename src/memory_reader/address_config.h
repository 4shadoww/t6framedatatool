#ifndef ADDRESS_CONFIG_H
#define ADDRESS_CONFIG_H

// TODO: Base address for different platforms

// Player 1 health
// Linux base address
#define GAME_BASE_ADDRESS 0x300B2C140
// Player 1 frames since last action
#define P1_FRAMES_LAST_ACTION GAME_BASE_ADDRESS + 124
// Player 1 frames since last connection
#define P1_FRAMES_LAST_CONNECTION GAME_BASE_ADDRESS + 915041796
// Plater 1 animation recovery frames
#define P1_RECOVERY_FRAMES GAME_BASE_ADDRESS + 4294967688

// Player 2 frames since last action
#define P2_FRAMES_LAST_ACTION GAME_BASE_ADDRESS + 3260
// Player 2 frames since last connection
#define P2_FRAMES_LAST_CONNECTION GAME_BASE_ADDRESS + 915050244
// Plater 2 animation recovery frames
#define P2_RECOVERY_FRAMES GAME_BASE_ADDRESS + 3528

// Current game frame
#define CURRENT_GAME_FRAME GAME_BASE_ADDRESS + 915040784

#endif
