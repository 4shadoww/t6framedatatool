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

#include <iostream>

#include "arg_parser.hpp"
#include "logging.h"

#include "frame_data_analyser.hpp"
#include "frame_dumper.h"
#include "version.hpp"

namespace {

class Listener : public EventListener {
public:
    void frame_data(FrameDataPoint frame_data) override;
    void distance(float /*distance*/) override;
    void status(PlayerState /*status*/) override;
    void game_hooked() override;
};

void Listener::frame_data(const FrameDataPoint frame_data) {
    const PlayerFrameData &p1 = frame_data.p1;
    const PlayerFrameData &p2 = frame_data.p2;

    std::cout << "player side: " << (frame_data.player_side == PlayerSide::LEFT ? "P1" : "P2") << std::endl
              << "P1 startup frames: " << p1.startup_frames << "(" << p1.first_active_frame << "~"
              << p1.last_active_frame << ")"
              << ", frame advantage: " << p1.frame_advantage << ", KD: " << p1.knock_down << std::endl
              << "P2 startup frames: " << p2.startup_frames << "(" << p2.first_active_frame << "~"
              << p2.last_active_frame << ")"
              << ", frame advantage: " << p2.frame_advantage << ", KD: " << p2.knock_down << std::endl;
}

void Listener::distance(const float /*distance*/) {
    // NOP
}

void Listener::status(const PlayerState /*status*/) {
    // NOP
}

void Listener::game_hooked() {
    // NOP
}

void apply_config(const Configuration &config) {
    log_set_level(config.log_level);
    FrameDataAnalyser::set_logging(config.frame_data_logging);
    set_memory_reader_logging(config.game_struct_logging);

    if (config.frame_dump_path != nullptr) {
        (void) frame_dump_start(config.frame_dump_path);
    }
}
} // namespace

int main(const int argc, const char *const *const argv) {
    Configuration config = ArgParser::create_default_config();
    const int result = ArgParser::parse_arguments(argc, argv, &config);
    if (result == 1) {
        return 1;
    } else if (result == -1) {
        // Early exit
        return 0;
    }
    apply_config(config);

    log_info("%s %s", PROGRAM_NAME, VERSION);

    Listener listener;
    FrameDataAnalyser::start(&listener);

    frame_dump_stop();

    return 0;
}
