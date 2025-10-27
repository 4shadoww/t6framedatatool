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

#include "logging.h"

#include "frame_data_analyser.hpp"
#include "version.hpp"

class Listener : public EventListener {
public:
    void frame_data(FrameDataPoint frame_data) override;
    void distance(float /*distance*/) override;
    void status(PlayerState /*status*/) override;
    void game_hooked() override;
};

void Listener::frame_data(const FrameDataPoint frame_data) {
    std::cout << "startup frames: " << frame_data.startup_frames << ", frame advantage: " << frame_data.frame_advantage
              << std::endl;
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

int main() {
    log_set_level(LOG_TRACE);

    log_info("%s %s", PROGRAM_NAME, VERSION);

    Listener listener;
    FrameDataAnalyser::start(&listener);

    return 0;
}
