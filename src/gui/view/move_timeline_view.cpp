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

#include "view/move_timeline_view.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <imgui.h> // NOLINT

#include "color_scheme.hpp"
#include "gui_constants.hpp"

namespace {

constexpr float WIDTH_SCALE = 0.6F;
constexpr float LABEL_WIDTH = 3.0F;
constexpr float ROW_HEIGHT = 3.0F;
constexpr float BAR_HEIGHT = 1.2F;
constexpr float PANEL_HEIGHT = 6.3F;
constexpr float HEADER_HEIGHT = 1.2F;
constexpr float SEGMENT_GAP = 0.12F;

float frame_x(const uint32_t frame, const uint32_t span, const float left, const float width) {
    const float ratio = span == 0 ? 0.0F : static_cast<float>(frame) / static_cast<float>(span);
    return left + (width * ratio);
}


} // namespace

RelativePanel MoveTimelineView::panel(const ImVec2 canvas) {
    return {
        .id = "##move_timeline",
        .horizontal = HorizontalAnchor::CENTER,
        .vertical = VerticalAnchor::TOP,
        .offset = {0.0F, ui::em(1.0F)},
        .size = {canvas.x * WIDTH_SCALE, ui::em(PANEL_HEIGHT)},
    };
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void MoveTimelineView::draw(const ImVec2 canvas, const MoveTimelineViewModel &viewModel) {
    OverlayCanvas::begin_panel(panel(canvas), canvas);

    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    const MoveTimelineModel model = viewModel.snapshot();
    ImDrawList *const draw_list = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float available = ImGui::GetContentRegionAvail().x;
    const float track_left = origin.x + ui::em(LABEL_WIDTH);
    const float track_width = std::max(0.0F, available - ui::em(LABEL_WIDTH));
    const float bar_height = ui::em(BAR_HEIGHT);
    const float row_height = ui::em(ROW_HEIGHT);

    // Normalize both players to the earliest displayed move
    uint32_t reference_start = 0;
    uint32_t span = 1;
    bool has_move = false;

    if (model.p1.active) {
        reference_start = model.p1.start_frame;
        has_move = true;
    }
    if (model.p2.active && (!has_move || model.p2.start_frame < reference_start)) {
        reference_start = model.p2.start_frame;
    }
    has_move = has_move || model.p2.active;

    if (has_move) {
        if (model.p1.active) {
            span = std::max(span, model.p1.end_frame - reference_start);
        }
        if (model.p2.active) {
            span = std::max(span, model.p2.end_frame - reference_start);
        }
    }


    const auto draw_timeline = [&](const MoveTimeline &timeline, const char *const label, const float row_y)
    {
        draw_list->AddText(ImVec2(origin.x, row_y), ImGui::GetColorU32(color_scheme::TEXT), label);
        draw_list->AddRectFilled(ImVec2(track_left, row_y),
                                 ImVec2(track_left + track_width, row_y + bar_height),
                                 ImGui::GetColorU32(color_scheme::FRAME_BACKGROUND));

        if (!timeline.active) {
            draw_list->AddText(ImVec2(track_left, row_y), ImGui::GetColorU32(color_scheme::MUTED_TEXT), "idle");
            return;
        }

        const uint32_t start = timeline.start_frame - reference_start;
        const uint32_t first_active = timeline.first_active_frame - reference_start;
        const uint32_t last_active = timeline.last_active_frame - reference_start;
        const uint32_t end = timeline.end_frame - reference_start;

        const float block_width = span > 0 ? track_width / static_cast<float>(span) : track_width;
        const float inset = std::min(ui::em(SEGMENT_GAP), block_width * 0.25F);
        const float bar_bottom = row_y + bar_height;
        const float label_width = ImGui::CalcTextSize("000").x + ui::em(0.1F);
        const auto label_step =
            std::max<uint32_t>(1U, static_cast<uint32_t>(std::ceil(label_width / std::max(1.0F, block_width))));

        for (uint32_t frame = 0; frame < span; frame++) {
            if (frame < start || frame >= end) {
                continue;
            }

            ImVec4 color;
            if (frame < first_active) {
                color = color_scheme::HIT_LEVEL_LOW;
            } else if (frame <= last_active) {
                color = color_scheme::NEGATIVE;
            } else {
                color = color_scheme::CORRECTION_MARKER;
            }

            const float x = frame_x(frame, span, track_left, track_width);
            const ImVec2 min(x + (inset * 0.5F), row_y);
            const ImVec2 max(x + block_width - (inset * 0.5F), bar_bottom);
            if (max.x <= min.x) {
                continue;
            }
            draw_list->AddRectFilled(min, max, ImGui::GetColorU32(color));
            draw_list->AddRect(min, max, color_scheme::OUTLINE, 0.0F, 0, ui::hairline());

            if (frame % label_step == 0) {
                char label[16] = {};
                (void) std::snprintf(label, sizeof(label), "%u", frame);
                const ImVec2 label_size = ImGui::CalcTextSize(label);
                const float center_x = (min.x + max.x) * 0.5F;
                const float label_x = std::clamp(center_x - (label_size.x * 0.5F),
                                                 track_left,
                                                 std::max(track_left, track_left + track_width - label_size.x));
                const float label_y = row_y - ImGui::GetFontSize();
                draw_list->AddText(ImVec2(label_x, label_y), ImGui::GetColorU32(color_scheme::TEXT_DIM), label);
            }
        }
    };

    const float bars_top = origin.y + ui::em(HEADER_HEIGHT);
    draw_timeline(model.p1, "P1", bars_top);
    draw_timeline(model.p2, "P2", bars_top + row_height);

    ImGui::PopFont();

    OverlayCanvas::end_panel();
}
