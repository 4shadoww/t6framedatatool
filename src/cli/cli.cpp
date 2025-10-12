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

class listener : public event_listener {
public:
    void frame_data(frame_data_point frame_data) override;
    void distance(float /*distance*/) override;
    void status(player_state /*status*/) override;
    void game_hooked() override;
};

void listener::frame_data(const frame_data_point frame_data) {
    std::cout << "startup frames: " << frame_data.startup_frames << ", frame advantage: " << frame_data.frame_advantage
              << std::endl;
}

void listener::distance(const float /*distance*/) {
    // NOP
}

void listener::status(const player_state /*status*/) {
    // NOP
}

void listener::game_hooked() {
    // NOP
}

int main() {
    log_set_level(LOG_TRACE);

    listener listener;
    frame_data_analyser::start(&listener);

    return 0;
}
