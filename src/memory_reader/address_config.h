#ifndef ADDRESS_CONFIG_H
#define ADDRESS_CONFIG_H

// TODO: connection boolean address may be dynamic (not confirmed)

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
// Player 1 coordinates
#define P1_POSITION GAME_BASE_ADDRESS + 2192

// Player 2 frames since last action
#define P2_FRAMES_LAST_ACTION GAME_BASE_ADDRESS + 3260
// Plater 2 animation recovery frames
#define P2_RECOVERY_FRAMES GAME_BASE_ADDRESS + 3528
// Plater 2 connection boolean
#define P2_CONNECTION_BOOL GAME_BASE_ADDRESS + 915132186
// Player 2 intent
#define P2_INTENT GAME_BASE_ADDRESS + 3608
// Player 2 coordinates
#define P2_POSITION GAME_BASE_ADDRESS + 5248

// Current game frame
#define CURRENT_GAME_FRAME GAME_BASE_ADDRESS + 270587472


// Dynamic data section base pointer
//#define GAME_DYN_DATA_PTR = 0x300C3F9E8
// Player 1 frames since last connection
//#define P1_FRAMES_LAST_CONNECTION_PTR_OFFSET 913922468

#endif
