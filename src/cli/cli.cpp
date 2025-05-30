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

void new_frame_data(struct frame_data_point data_point) {
    std::cout << "startup frames: " << data_point.startup_frames << ", frame advantage: " <<
                 data_point.frame_advantage << std::endl;
}

int main(const int argc, const char **argv) {
    log_set_level(LOG_TRACE);
    frame_data_analyser::start(&new_frame_data);

    return 0;
}
