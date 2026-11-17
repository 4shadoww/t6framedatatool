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

#include "view/configuration_button_view.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

#include <imgui.h> // NOLINT

#include "color_scheme.hpp"
#include "gui_constants.hpp"

namespace {

void draw_gear(ImDrawList *const draw_list, const ImVec2 center, const float radius, const ImU32 color) {
    constexpr int TEETH = 8;
    constexpr float INNER_RADIUS_SCALE = 0.72F;
    constexpr float TOOTH_WIDTH_SCALE = 0.18F;
    constexpr float HOLE_RADIUS_SCALE = 0.34F;
    constexpr float OUTLINE_SCALE = 0.12F;

    for (int i = 0; i < TEETH; i++) {
        const float angle = (2.0F * std::numbers::pi_v<float> * static_cast<float>(i)) / static_cast<float>(TEETH);
        const ImVec2 inner(center.x + (std::cos(angle) * radius * INNER_RADIUS_SCALE),
                           center.y + (std::sin(angle) * radius * INNER_RADIUS_SCALE));
        const ImVec2 outer(center.x + (std::cos(angle) * radius), center.y + (std::sin(angle) * radius));
        draw_list->AddLine(inner, outer, color, radius * TOOTH_WIDTH_SCALE);
    }

    draw_list->AddCircle(center, radius * INNER_RADIUS_SCALE, color, 0, radius * OUTLINE_SCALE);
    draw_list->AddCircle(center, radius * HOLE_RADIUS_SCALE, color, 0, radius * OUTLINE_SCALE);
}

} // namespace

RelativePanel ConfigurationButtonView::panel(const ImVec2 /*canvas*/) {
    constexpr float SIZE = ui::em(2.0F);
    constexpr float OFFSET = 0.0F;

    return {
        .id = "##configuration_button",
        .horizontal = HorizontalAnchor::RIGHT,
        .vertical = VerticalAnchor::TOP,
        .offset = {OFFSET, OFFSET},
        .size = {SIZE, SIZE},
        .border_size = 0.0F,
    };
}

void ConfigurationButtonView::draw(const ImVec2 canvas, ConfigurationViewModel &viewModel) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
    ImGui::PushStyleColor(ImGuiCol_Button, color_scheme::WINDOW_BACKGROUND);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_scheme::BUTTON_HOVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color_scheme::BUTTON_ACTIVE);
    OverlayCanvas::begin_panel(panel(canvas), canvas);

    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    const ImVec2 button_size = ImGui::GetContentRegionAvail();
    if (ImGui::Button("##config", button_size)) {
        viewModel.set_open(true);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Config");
    }

    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    const ImVec2 center((min.x + max.x) * 0.5F, (min.y + max.y) * 0.5F);
    const float radius = std::min(max.x - min.x, max.y - min.y) * 0.3F;
    draw_gear(ImGui::GetWindowDrawList(), center, radius, ImGui::GetColorU32(ImGuiCol_Text));

    ImGui::PopFont();

    OverlayCanvas::end_panel();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

bool ConfigurationButtonView::contains(const ImVec2 canvas, const ImVec2 point) {
    const OverlayPanel resolved = OverlayCanvas::resolve(panel(canvas), canvas);

    return point.x >= resolved.position.x && point.x <= resolved.position.x + resolved.size.x &&
           point.y >= resolved.position.y && point.y <= resolved.position.y + resolved.size.y;
}
