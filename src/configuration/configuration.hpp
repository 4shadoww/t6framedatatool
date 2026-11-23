/*
  Copyright (C) 2026 Noa-Emil Nissinen

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

#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

constexpr float MIN_UI_SCALE = 0.5F;
constexpr float MAX_UI_SCALE = 1.5F;

struct ConfigurationData {
    struct Ui {
        bool show_command_history = true;
        bool show_player2_command_history = false;
        bool show_own_frame_data = true;
        bool show_opponent_frame_data = true;
        bool show_only_in_practice = false;
        bool disable_config_button = false;
        bool show_move_timeline = false;
        float scale = 1.0F;
    };

    Ui ui;
};

#endif
