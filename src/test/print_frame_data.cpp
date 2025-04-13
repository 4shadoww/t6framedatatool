#include <iostream>

extern "C" {
#include "memory_reader.h"
}

// Just print the current frame data and exit

int main(const int argc, const char **argv) {
    init_memory_reader();

    std::cout << "FRAME DATA" << std::endl;
    std::cout << "----------" << std::endl;

    // P1
    std::cout << "P1 last action: " << p1_frames_last_action() << std::endl;
    std::cout << "P1 connection: " << p1_connection() << std::endl;
    std::cout << "P1 recovery frames: " << p1_recovery_frames() << std::endl;

    // P2
    std::cout << "P2 last action: " << p2_frames_last_action() << std::endl;
    std::cout << "P2 connection: " << p2_connection() << std::endl;
    std::cout << "P2 recovery frames: " << p2_recovery_frames() << std::endl;

    // Game state
    std::cout << "current game frame: " << current_game_frame() << std::endl;

    return 0;
}
