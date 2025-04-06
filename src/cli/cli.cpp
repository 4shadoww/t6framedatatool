#include <iostream>

extern "C" {
#include "logging.h"
}

#include "frame_data_analyser.hpp"

void new_frame_data(struct frame_data_point data_point) {
    std::cout << "move frames: " << data_point.move_frames << ", frame advantage: " <<
                 data_point.frame_advantage << std::endl;
}

int main(const int argc, const char **argv) {
    log_set_level(LOG_TRACE);
    frame_data_analyser::start(&new_frame_data);

    return 0;
}
