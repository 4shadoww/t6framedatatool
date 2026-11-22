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

    const float content_padding = ui::em(2.0F);
    const ImVec2 content_padding_y = ui::em(0.0F, 0.5F);

    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    if (ImGui::BeginTabBar("##configuration_tabs")) {
        if (ImGui::BeginTabItem("User Interface")) {
            ImGui::Dummy(content_padding_y);
            ImGui::Indent(content_padding);
            UiConfigView::draw(viewModel);
            ImGui::Unindent(content_padding);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Frame Data")) {
            ImGui::Dummy(content_padding_y);
            ImGui::Indent(content_padding);
            FrameDataConfigView::draw(viewModel);
            ImGui::Unindent(content_padding);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    const float close_button_size = ui::em(1.5F);
    const ImVec2 content_max = ImGui::GetWindowContentRegionMax();
    ImGui::SetCursorPos(ImVec2(content_max.x - close_button_size, 0.0F));
    if (ImGui::Button("X", ImVec2(close_button_size, close_button_size))) {
        viewModel.set_open(false);
    }

    const float save_button_width = ui::em(4.0F);
    const float save_button_height = ui::em(1.5F);
    const float save_button_padding = ui::em(1.0F);
    const ImVec2 content_min = ImGui::GetWindowContentRegionMin();
    ImGui::SetCursorPos(
        ImVec2(content_min.x + save_button_padding, content_max.y - save_button_height - save_button_padding));
    if (ImGui::Button("Save", ImVec2(save_button_width, save_button_height))) {
        (void) viewModel.save();
    }

    if (viewModel.toast_visible()) {
        const char *const text = viewModel.toast_message().c_str();
        const ImVec2 text_size = ImGui::CalcTextSize(text);
        const ImVec2 window_position = ImGui::GetWindowPos();
        const ImVec2 window_size = ImGui::GetWindowSize();
        const ImVec2 text_position(window_position.x + ((window_size.x - text_size.x) * 0.5F),
                                   window_position.y + ((window_size.y - text_size.y) * 0.5F));

        ImGui::GetWindowDrawList()->AddText(text_position, ImGui::GetColorU32(ImGuiCol_TextDisabled), text);
    }

    ImGui::PopFont();

    OverlayCanvas::end_panel();
}
