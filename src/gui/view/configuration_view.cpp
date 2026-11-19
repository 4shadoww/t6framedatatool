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

#include "view/configuration_view.hpp"

#include <imgui.h> // NOLINT

#include "gui_constants.hpp"
#include "view/configuration/frame_data_config_view.hpp"
#include "view/configuration/ui_config_view.hpp"

namespace {

constexpr float CONTENT_PADDING = ui::em(2.0F);
// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
const ImVec2 content_padding_y = ui::em(0.0F, 0.5F);

} // namespace
//
RelativePanel ConfigurationView::panel(const ImVec2 canvas) {
    constexpr float WIDTH_SCALE = 0.8F;
    constexpr float HEIGHT_SCALE = 0.8F;

    return {
        .id = "##configuration",
        .horizontal = HorizontalAnchor::CENTER,
        .vertical = VerticalAnchor::CENTER,
        .offset = {0.0F, 0.0F},
        .size = {canvas.x * WIDTH_SCALE, canvas.y * HEIGHT_SCALE},
    };
}

void ConfigurationView::draw(const ImVec2 canvas, ConfigurationViewModel &viewModel) {
    if (!viewModel.is_open()) {
        // Not open nothing to draw
        return;
    }
    OverlayCanvas::begin_panel(panel(canvas), canvas);

    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    if (ImGui::BeginTabBar("##configuration_tabs")) {
        if (ImGui::BeginTabItem("User Interface")) {
            ImGui::Dummy(content_padding_y);
            ImGui::Indent(CONTENT_PADDING);
            UiConfigView::draw(viewModel);
            ImGui::Unindent(CONTENT_PADDING);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Frame Data")) {
            ImGui::Dummy(content_padding_y);
            ImGui::Indent(CONTENT_PADDING);
            FrameDataConfigView::draw(viewModel);
            ImGui::Unindent(CONTENT_PADDING);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    constexpr float CLOSE_BUTTON_SIZE = ui::em(1.5F);
    const ImVec2 content_max = ImGui::GetWindowContentRegionMax();
    ImGui::SetCursorPos(ImVec2(content_max.x - CLOSE_BUTTON_SIZE, 0.0F));
    if (ImGui::Button("X", ImVec2(CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE))) {
        viewModel.set_open(false);
    }

    ImGui::PopFont();

    OverlayCanvas::end_panel();
}
