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
#ifndef PLATFORM_GUI_HPP
#define PLATFORM_GUI_HPP

struct GLFWwindow;

bool platform_make_overlay(GLFWwindow *window);
void platform_update_ui_position(const int game_x, const int game_y, const int game_height, const bool margin);
void platform_find_game_window();
void platform_update();

#endif
