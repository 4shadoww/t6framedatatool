#ifndef FRAME_DATA_ANALYSER_HPP
#define FRAME_DATA_ANALYSER_HPP

#include "ring_buffer.hpp"

extern "C" {
#include "memory_reader.h"
}

struct frame_data_point {
    int startup_frames;
    int frame_advantage;
};

struct start_frame {
    size_t index;
    uint32_t recovery_frames;
    uint32_t game_frame;
};

enum connection_event {
    NO_CONNECTION,
    P1_CONNECTION,
    P2_CONNECTION
};

enum player_intents : int {
    IDLE = 0,
    ATTACK1 = 1,
    ATTACK3 = 3,
    ATTACK5 = 5,
    ATTACK7 = 7, // Usually string
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
    static ring_buffer<game_state> m_frame_buffer;
    static void (*callback)(struct frame_data_point);

    // Analysis state
    static ring_buffer<start_frame> m_p1_start_frames;
    static ring_buffer<start_frame> m_p2_start_frames;


    static bool init(void (*callback)(struct frame_data_point));
    static bool loop();

    inline static bool is_attack(const int action);
    static start_frame get_startup_frame(const bool p2);

    static void analyse_start_frames();
    static bool update_game_state();
    static connection_event has_new_connection();
    static void handle_connection();
};

#endif
