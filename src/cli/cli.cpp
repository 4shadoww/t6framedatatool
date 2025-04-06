extern "C" {
#include "logging.h"
}

#include "frame_data_analyser.hpp"

int main(const int argc, const char **argv) {
    log_set_level(LOG_TRACE);
    frame_data_analyser::start();

    return 0;
}
