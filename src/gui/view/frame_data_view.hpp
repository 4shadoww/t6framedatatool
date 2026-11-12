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

#ifndef FRAME_DATA_PANEL_HPP
#define FRAME_DATA_PANEL_HPP

#include "component/overlay_canvas.hpp"
#include "viewmodel/frame_data_view_model.hpp"


class FrameDataPanel {
public:
    static void draw(const ImVec2 canvas, const FrameDataViewModel &viewModel);

private:
    static RelativePanel panel(const ImVec2 canvas);
};

#endif
