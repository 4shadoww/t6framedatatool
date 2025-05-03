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
#define MR_INIT_ALREADY_DONE -2

struct player_coordinate {
    float x;
    float y;
    float z;
};

struct game_state {
    uint32_t game_frame;

    uint32_t p1_frames_last_action;
    uint32_t p1_recovery_frames;
    uint8_t p1_connection;
    uint32_t p1_intent;
    struct player_coordinate p1_position;

    uint32_t p2_frames_last_action;
    uint32_t p2_recovery_frames;
    uint8_t p2_connection;
    uint32_t p2_intent;
    struct player_coordinate p2_position;
};

#define READ_ERROR (uint32_t) -1

/**
* Finds T6 process ID and initializes the memory
*
* @return MR_INIT value
*/
int init_memory_reader(void);

uint32_t p1_frames_last_action(void);
uint32_t p1_connection(void);
uint32_t p1_recovery_frames(void);
uint32_t p1_intent(void);
struct player_coordinate p1_position(void);

uint32_t p2_frames_last_action(void);
uint32_t p2_connection(void);
uint32_t p2_recovery_frames(void);
uint32_t p2_intent(void);
struct player_coordinate p2_position(void);

uint32_t current_game_frame(void);

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
