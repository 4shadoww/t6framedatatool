#ifndef FRAME_DATA_ANALYSER_HPP
#define FRAME_DATA_ANALYSER_HPP

extern "C" {
#include "memory_reader.h"
}

struct frame_data_point {
    int move_frames;
    int frame_advantage;
};

struct player_attack_frames {
    int p1_last_attack_frame;
    int p2_last_attack_frame;
};

enum player_intents : int {
    IDLE = 0,
    ATTACK1 = 1,
    ATTACK3 = 3,
    ATTACK5 = 5,
    ATTACK7 = 7,
    INPUT_BUFFERING = 10,
    BLOCK = 11,
    WALK = 12,
    SIDE_STEP = 15,
    STASIS = 16,
    DOUBLE_SIDE_STEP = 28,
    GRAP_INIT = 65539,
    GRAP_CONNECT = 65546,
};

class frame_data_analyser {
public:
    /*
    * Hook-up to game's memory and start analysing frames
    *
    * @param callback callback pointer
    */
    static bool start(void (*callback)(struct frame_data_point));

private:
    static struct game_state m_state;
    static struct game_state m_previous_state;
    static struct player_attack_frames m_attack_frames;

    static void (*callback)(struct frame_data_point);

    static bool init(void (*callback)(struct frame_data_point));
    static bool loop();

    static bool is_attack(const int action);

    static void update_attack_init_frames();
    static bool has_new_connection();
    inline static void update_last_connection();
};

#endif
