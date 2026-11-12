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

#include "overlay_canvas.hpp"

ImVec2 OverlayCanvas::size() {
    return ImGui::GetMainViewport()->WorkSize;
}

OverlayPanel OverlayCanvas::resolve(const RelativePanel &panel, const ImVec2 canvas) {
    float x = 0.0F;
    float y = 0.0F;

    switch (panel.horizontal) {
    case HorizontalAnchor::LEFT:
        x = panel.offset.x;
        break;
    case HorizontalAnchor::CENTER:
        x = ((canvas.x - panel.size.x) * 0.5F) + panel.offset.x;
        break;
    case HorizontalAnchor::RIGHT:
        x = canvas.x - panel.size.x - panel.offset.x;
        break;
    }

    switch (panel.vertical) {
    case VerticalAnchor::TOP:
        y = panel.offset.y;
        break;
    case VerticalAnchor::CENTER:
        y = ((canvas.y - panel.size.y) * 0.5F) + panel.offset.y;
        break;
    case VerticalAnchor::BOTTOM:
        y = canvas.y - panel.size.y - panel.offset.y;
        break;
    }

    return {
        .id = panel.id,
        .size = panel.size,
        .position = ImVec2(x, y),
    };
}

void OverlayCanvas::begin_panel(const OverlayPanel &panel) {
    ImGui::SetNextWindowPos(panel.position);
    ImGui::SetNextWindowSize(panel.size);
    ImGui::SetNextWindowBgAlpha(0.92F);

    const unsigned int flag_bits = static_cast<unsigned int>(ImGuiWindowFlags_NoDecoration) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoMove) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoResize) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoSavedSettings) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoBringToFrontOnFocus) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoNavFocus) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoScrollbar) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoScrollWithMouse);
    const auto flags = static_cast<ImGuiWindowFlags>(flag_bits);

    ImGui::Begin(panel.id, nullptr, flags);
}

void OverlayCanvas::begin_panel(const RelativePanel &panel, const ImVec2 canvas) {
    begin_panel(resolve(panel, canvas));
}

void OverlayCanvas::end_panel() {
    ImGui::End();
}
