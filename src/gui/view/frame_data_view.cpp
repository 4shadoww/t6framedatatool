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
#include "frame_data_defs.hpp"
#include "gui_constants.hpp"
#include "logging.h"

namespace {

constexpr float LABEL_COLUMN_WIDTH = ui::em(10.0F);
constexpr ImVec4 POSITIVE_COLOR = ImVec4(0.35F, 1.0F, 0.55F, 1.0F);
constexpr ImVec4 NEGATIVE_COLOR = ImVec4(1.0F, 0.35F, 0.35F, 1.0F);

const PlayerFrameData &player_data(const FrameDataPoint &data, const PlayerSide side) {
    return side == PlayerSide::LEFT ? data.p1 : data.p2;
}

struct HitLevelStyle {
    const char *name = nullptr;
    ImVec4 color;
};

HitLevelStyle hit_level_style(const HitLevel level) {
    switch (level) {
    case HitLevel::HIGH:
    case HitLevel::THROW:
        return {.name = "High", .color = ImVec4(1.0F, 0.30F, 0.30F, 1.0F)};
    case HitLevel::UNBLOCKABLE_HIGH:
        return {.name = "Unblockable High", .color = ImVec4(1.0F, 0.30F, 0.30F, 1.0F)};
    case HitLevel::MID:
        return {.name = "Mid", .color = ImVec4(1.0F, 0.85F, 0.25F, 1.0F)};
    case HitLevel::LOW:
        return {.name = "Low", .color = ImVec4(0.35F, 0.65F, 1.0F, 1.0F)};
    case HitLevel::SPECIAL_LOW:
        return {.name = "Special Low", .color = ImVec4(1.0F, 0.55F, 0.20F, 1.0F)};
    case HitLevel::SPECIAL_MID:
        return {.name = "Special Mid", .color = ImVec4(0.35F, 0.90F, 0.90F, 1.0F)};
    default:
        log_warn("unknown hit level %d", level);
        [[fallthrough]];
    case HitLevel::NONE:
        return {.name = "---", .color = ImVec4(0.70F, 0.70F, 0.70F, 1.0F)};
    }
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

    draw_label("Hit Level");
    ImGui::TextUnformatted("---");

    draw_label("Damage");
    ImGui::TextUnformatted("---");

    draw_label("Frame Advantage");
    ImGui::TextUnformatted("---");

    draw_label("Status");
    ImGui::TextUnformatted("---");

    draw_label("Distance");
    ImGui::TextUnformatted("---");

    ImGui::PopFont();
}

void draw_no_frame_data(const FrameDataViewModel &viewModel, const PlayerSide side) {
    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    draw_label("Startup Frames");
    ImGui::TextUnformatted("---");

    draw_label("Hit Level");
    ImGui::TextUnformatted("---");

    draw_label("Damage");
    ImGui::TextUnformatted("---");

    draw_label("Frame Advantage");
    ImGui::TextUnformatted("---");

    draw_label("Status");
    ImGui::TextUnformatted(FrameDataAnalyser::player_status(viewModel.get_player_state(side)));

    draw_label("Distance");
    ImGui::Text("%.2f", viewModel.get_distance());

    ImGui::PopFont();
}

void draw_frame_data(const FrameDataViewModel &viewModel, const PlayerSide side) {
    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    const FrameDataPoint frame_data_point = viewModel.get_frame_data();
    const PlayerFrameData &data = player_data(frame_data_point, side);
    const bool knock_down = data.knock_down;

    draw_label("Startup Frames");
    ImGui::Text("%d (%u~%u)", data.startup_frames, data.first_active_frame, data.last_active_frame);

    draw_label("Hit Level");
    const HitLevelStyle hit_level = hit_level_style(data.hit_level);
    ImGui::TextColored(hit_level.color, "%s", hit_level.name);

    draw_label("Damage");
    ImGui::Text("%u", data.damage);

    draw_label("Frame Advantage");
    if (knock_down) {
        ImGui::TextColored(data.is_attacker ? POSITIVE_COLOR : NEGATIVE_COLOR, "KD");
    } else {
        draw_advantage(data.frame_advantage);
    }

    draw_label("Status");
    ImGui::TextUnformatted(FrameDataAnalyser::player_status(viewModel.get_player_state(side)));

    draw_label("Distance");
    ImGui::Text("%.2f", viewModel.get_distance());

    ImGui::PopFont();
}

} // namespace

RelativePanel FrameDataView::panel(const ImVec2 /*canvas*/, const PlayerSide side) {
    constexpr float SIZE_X = ui::em(19.0F);
    constexpr float SIZE_Y = ui::em(8.0F);

    constexpr float PADDING_SIDE = ui::em(8.0F);
    constexpr float PADDING_BOTTOM = ui::em(4.0F);

    return {
        .id = side == PlayerSide::LEFT ? "##frame_data_p1" : "##frame_data_p2",
        .horizontal = side == PlayerSide::LEFT ? HorizontalAnchor::LEFT : HorizontalAnchor::RIGHT,
        .vertical = VerticalAnchor::BOTTOM,
        .offset = {PADDING_SIDE, PADDING_BOTTOM},
        .size = {SIZE_X, SIZE_Y},
    };
}

void FrameDataView::draw(const ImVec2 canvas, const FrameDataViewModel &viewModel) {
    draw_panel(canvas, viewModel, PlayerSide::LEFT);
    draw_panel(canvas, viewModel, PlayerSide::RIGHT);
}

void FrameDataView::draw_panel(const ImVec2 canvas, const FrameDataViewModel &viewModel, const PlayerSide side) {
    OverlayCanvas::begin_panel(panel(canvas, side), canvas);

    if (!viewModel.is_game_hooked()) {
        draw_no_game();
    } else if (!viewModel.has_frame_data()) {
        draw_no_frame_data(viewModel, side);
    } else {
        draw_frame_data(viewModel, side);
    }

    OverlayCanvas::end_panel();
}
