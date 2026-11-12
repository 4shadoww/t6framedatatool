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

#include "view/frame_data_view.hpp"

#include <imgui.h> // NOLINT

#include "frame_data_analyser.hpp"
#include "gui_constants.hpp"

namespace {

constexpr float LABEL_COLUMN_WIDTH = 210.0F;
constexpr ImVec4 POSITIVE_COLOR = ImVec4(0.35F, 1.0F, 0.55F, 1.0F);
constexpr ImVec4 NEGATIVE_COLOR = ImVec4(1.0F, 0.35F, 0.35F, 1.0F);
constexpr ImVec4 KNOCKDOWN_COLOR = ImVec4(1.0F, 0.75F, 0.25F, 1.0F);

const PlayerFrameData &local_player_data(const FrameDataPoint &data) {
    return data.player_side == PlayerSide::LEFT ? data.p1 : data.p2;
}

void draw_label(const char *const label) {
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine(LABEL_COLUMN_WIDTH);
}

void draw_advantage(const int advantage) {
    ImGui::TextColored(advantage < 0 ? NEGATIVE_COLOR : POSITIVE_COLOR, "%+d", advantage);
}

void draw_no_game() {
    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    draw_label("Startup Frames");
    ImGui::TextUnformatted("--- (No game hooked)");

    draw_label("Frame Advantage");
    ImGui::TextUnformatted("---");

    draw_label("Status");
    ImGui::TextUnformatted("---");

    draw_label("Distance");
    ImGui::TextUnformatted("---");

    ImGui::PopFont();
}

void draw_no_frame_data(const FrameDataViewModel &viewModel) {
    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    draw_label("Startup Frames");
    ImGui::TextUnformatted("---");

    draw_label("Frame Advantage");
    ImGui::TextUnformatted("---");

    draw_label("Status");
    ImGui::TextUnformatted(FrameDataAnalyser::player_status(viewModel.get_player_state()));

    draw_label("Distance");
    ImGui::Text("%.2f", viewModel.get_distance());

    ImGui::PopFont();
}

void draw_frame_data(const FrameDataViewModel &viewModel) {
    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    const FrameDataPoint frame_data_point = viewModel.get_frame_data();
    const PlayerFrameData &data = local_player_data(frame_data_point);
    const bool knock_down = frame_data_point.p1.knock_down || frame_data_point.p2.knock_down;

    draw_label("Startup Frames");
    ImGui::Text("%d (%u~%u)", data.startup_frames, data.first_active_frame, data.last_active_frame);

    draw_label("Frame Advantage");
    if (knock_down) {
        ImGui::TextColored(KNOCKDOWN_COLOR, "KD");
    } else {
        draw_advantage(data.frame_advantage);
    }

    draw_label("Status");
    ImGui::TextUnformatted(FrameDataAnalyser::player_status(viewModel.get_player_state()));

    draw_label("Distance");
    ImGui::Text("%.2f", viewModel.get_distance());

    ImGui::PopFont();
}

} // namespace

RelativePanel FrameDataPanel::panel(const ImVec2 /*canvas*/) {
    constexpr float SIZE_X = 400;
    constexpr float SIZE_Y = 150;

    constexpr float PADDING_LEFT = 200;
    constexpr float PADDING_BOTTOM = 100;

    return {
        .id = "##frame_data",
        .horizontal = HorizontalAnchor::LEFT,
        .vertical = VerticalAnchor::BOTTOM,
        .offset = {PADDING_LEFT, PADDING_BOTTOM},
        .size = {SIZE_X, SIZE_Y},
    };
}

void FrameDataPanel::draw(const ImVec2 canvas, const FrameDataViewModel &viewModel) {
    OverlayCanvas::begin_panel(panel(canvas), canvas);

    if (!viewModel.is_game_hooked()) {
        draw_no_game();
    } else if (!viewModel.has_frame_data()) {
        draw_no_frame_data(viewModel);
    } else {
        draw_frame_data(viewModel);
    }

    OverlayCanvas::end_panel();
}
