#ifndef FRAME_DATA_ANALYSER_HPP
#define FRAME_DATA_ANALYSER_HPP

extern "C" {
#include "memory_reader.h"
}

class frame_data_analyser {
public:
    /*
    * Hook-up to game's memory and start analysing frames
    */
    static bool start();

private:
    // Last updates game frame
    static uint32_t m_last_frame;
    // Last connection game frame
    static uint32_t m_last_connection;

    static struct game_state m_state;

    static bool init();
    static bool loop();

    static bool has_new_connection();
    inline static void update_last_connection();
};

#endif
