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

#ifndef CONFIGURATION_BUTTON_VIEW_HPP
#define CONFIGURATION_BUTTON_VIEW_HPP

#include "component/overlay_canvas.hpp"
#include "viewmodel/configuration_view_model.hpp"

class ConfigurationButtonView {
public:
    static void draw(ImVec2 canvas, ConfigurationViewModel &viewModel);
    static bool contains(ImVec2 canvas, ImVec2 point);

private:
    static RelativePanel panel(ImVec2 canvas);
};

#endif
