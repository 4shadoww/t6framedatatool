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

#include "view/command_history_view.hpp"

#include "gui_constants.hpp"

namespace {} // namespace

RelativePanel CommandHistoryView::panel(const ImVec2 /*canvas*/) {
    constexpr float SIZE_X = 250.0F;
    constexpr float SIZE_Y = 500.0F;
    constexpr float PADDING_LEFT = 50;
    return {
        .id = "##command_history",
        .horizontal = HorizontalAnchor::LEFT,
        .vertical = VerticalAnchor::CENTER,
        .offset = {PADDING_LEFT, 0.0F},
        .size = {SIZE_X, SIZE_Y},
    };
}

void CommandHistoryView::draw(const ImVec2 canvas) {
    OverlayCanvas::begin_panel(panel(canvas), canvas);

    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);
    ImGui::TextDisabled("Command History");
    ImGui::Separator();
    ImGui::TextDisabled("No input history");
    ImGui::PopFont();

    OverlayCanvas::end_panel();
}
