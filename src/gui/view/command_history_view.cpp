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

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "color_scheme.hpp"
#include "gui_constants.hpp"

namespace {

constexpr float EDGE_PADDING_FACTOR = 2.0F;
constexpr float SIZE_X_FACTOR = 6.0F;
constexpr float SIZE_Y_FACTOR = 38.0F;

void draw_outlined_text(ImDrawList *const draw_list,
                        const ImVec2 position,
                        const ImU32 color,
                        const char *const text,
                        const float outline) {
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            if (x == 0 && y == 0) {
                continue;
            }

            draw_list->AddText(
                ImVec2(position.x + (static_cast<float>(x) * outline), position.y + (static_cast<float>(y) * outline)),
                color_scheme::OUTLINE,
                text);
        }
    }

    draw_list->AddText(position, color, text);
}

void draw_arrow(ImDrawList *const draw_list,
                const ImVec2 center,
                const float radius,
                const uint8_t dirs,
                const ImU32 color,
                const float outline) {
    constexpr float NEUTRAL_RADIUS = 0.18F;
    constexpr float TAIL_SCALE = 0.8F;
    constexpr float BASE_SCALE = 0.15F;
    constexpr float SHAFT_SCALE = 0.32F;
    constexpr float HEAD_SCALE = 0.55F;

    float x = ((dirs & 8U) != 0U ? 1.0F : 0.0F) - ((dirs & 4U) != 0U ? 1.0F : 0.0F);
    float y = ((dirs & 2U) != 0U ? 1.0F : 0.0F) - ((dirs & 1U) != 0U ? 1.0F : 0.0F);

    if (x == 0.0F && y == 0.0F) {
        draw_list->AddCircleFilled(center, (radius * NEUTRAL_RADIUS) + outline, color_scheme::OUTLINE);
        draw_list->AddCircleFilled(center, radius * NEUTRAL_RADIUS, color);
        return;
    }

    const float length = std::sqrt((x * x) + (y * y));
    x /= length;
    y /= length;

    const ImVec2 tip(center.x + (x * radius), center.y + (y * radius));
    const ImVec2 tail(center.x - (x * radius * TAIL_SCALE), center.y - (y * radius * TAIL_SCALE));
    const ImVec2 base(center.x + (x * radius * BASE_SCALE), center.y + (y * radius * BASE_SCALE));

    draw_list->AddLine(tail, base, color_scheme::OUTLINE, (radius * SHAFT_SCALE) + (outline * 2.0F));

    const float width = radius * HEAD_SCALE;
    const ImVec2 left(base.x - (y * width), base.y + (x * width));
    const ImVec2 right(base.x + (y * width), base.y - (x * width));

    draw_list->AddLine(tip, left, color_scheme::OUTLINE, outline * 2.0F);
    draw_list->AddLine(tip, right, color_scheme::OUTLINE, outline * 2.0F);

    draw_list->AddLine(tail, base, color, radius * SHAFT_SCALE);
    draw_list->AddTriangleFilled(tip, left, right, color);
}

void draw_buttons(ImDrawList *const draw_list,
                  const ImVec2 position,
                  const float size,
                  const uint8_t buttons,
                  const float outline) {
    constexpr float BUTTON_OFFSET = 0.24F;
    constexpr float BUTTON_RADIUS = 0.19F;
    constexpr uint32_t BUTTON_COUNT = 4U;
    const float offset = size * BUTTON_OFFSET;
    const float radius = size * BUTTON_RADIUS;

    for (uint32_t i = 0; i < BUTTON_COUNT; i++) {
        const ImVec2 center(position.x + ((i & 1U) != 0 ? offset : -offset),
                            position.y + ((i & 2U) != 0 ? offset : -offset));

        if ((buttons & (1U << i)) != 0U) {
            draw_list->AddCircleFilled(center, radius + outline, color_scheme::OUTLINE);
            draw_list->AddCircleFilled(center, radius, color_scheme::INPUT_BUTTON_COLORS.at(i));
        } else {
            draw_list->AddCircle(center, radius, color_scheme::BUTTON_INACTIVE_OUTLINE, 0, ui::hairline());
            draw_list->AddCircle(center, radius, color_scheme::BUTTON_INACTIVE, 0, ui::hairline());
        }
    }
}

void draw_input_row(const CommandHistoryEntry &entry, const ImU32 text_color) {
    constexpr float ROW_HEIGHT_SCALE = 1.25F;
    constexpr float ROW_CENTER_SCALE = 0.5F;
    constexpr float ARROW_RADIUS_SCALE = 0.38F;
    constexpr float ARROW_CENTER_X = 0.75F;
    constexpr float ARROW_TO_BUTTONS = 1.05F;
    constexpr float BUTTON_OFFSET = 0.24F;
    constexpr float BUTTON_RADIUS = 0.19F;
    constexpr float BUTTONS_RIGHT = BUTTON_OFFSET + BUTTON_RADIUS;
    constexpr float FRAME_GAP = 0.50F;

    ImDrawList *const draw_list = ImGui::GetWindowDrawList();
    const ImVec2 position = ImGui::GetCursorScreenPos();
    const float height = ImGui::GetFontSize() * ROW_HEIGHT_SCALE;
    const float radius = height * ARROW_RADIUS_SCALE;

    char frames[8] = {};
    (void) std::snprintf(frames, sizeof(frames), "%u", entry.frames);

    const float center_y = position.y + (height * ROW_CENTER_SCALE);
    const float outline = std::max(ui::hairline(), height * 0.05F);
    float cursor_x = position.x + (height * ARROW_CENTER_X);

    draw_arrow(draw_list,
               ImVec2(cursor_x, center_y),
               radius,
               static_cast<uint8_t>(entry.input.direction),
               text_color,
               outline);
    cursor_x += height * ARROW_TO_BUTTONS;

    draw_buttons(draw_list, ImVec2(cursor_x, center_y), height, entry.input.buttons, outline);
    cursor_x += (height * BUTTONS_RIGHT) + (height * FRAME_GAP);

    const float frame_width = ImGui::CalcTextSize(frames).x;
    const ImVec2 frame_text_position(cursor_x, position.y + ((height - ImGui::GetFontSize()) * ROW_CENTER_SCALE));
    draw_outlined_text(draw_list, frame_text_position, text_color, frames, outline);

    ImGui::Dummy(ImVec2((cursor_x - position.x) + frame_width, height));
}

} // namespace

RelativePanel CommandHistoryView::panel(const ImVec2 canvas, const PlayerSide player, const PlayerSide screen_side) {
    const float margin = ui::em(EDGE_PADDING_FACTOR);
    const float available_x = std::max(0.0F, canvas.x - (margin * 2.0F));
    const float available_y = std::max(0.0F, canvas.y - (margin * 2.0F));
    const float size_x = std::min(ui::em(SIZE_X_FACTOR), available_x);
    const float size_y = std::min(ui::em(SIZE_Y_FACTOR), available_y);
    return {
        .id = player == PlayerSide::LEFT ? "##command_history_p1" : "##command_history_p2",
        .horizontal = screen_side == PlayerSide::LEFT ? HorizontalAnchor::LEFT : HorizontalAnchor::RIGHT,
        .vertical = VerticalAnchor::TOP,
        .offset = {margin, margin},
        .size = {size_x, size_y},
        .background_alpha = 0.0F,
        .border_size = 0.0F,
    };
}

void CommandHistoryView::draw(const ImVec2 canvas, const CommandHistoryViewModel &viewModel) {
    for (const PlayerSide player : {PlayerSide::LEFT, PlayerSide::RIGHT}) {
        if (!viewModel.show_command_history(player)) {
            continue;
        }

        OverlayCanvas::begin_panel(panel(canvas, player, viewModel.panel_side(player)), canvas);

        ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

        const CommandHistorySnapshot history = viewModel.snapshot(player);
        if (history.count == 0) {
            ImGui::TextDisabled("No history");
        } else {
            for (std::size_t i = history.count; i-- > 0;) {
                const ImU32 text_color = i == history.count - 1 ? color_scheme::TEXT_BRIGHT : color_scheme::TEXT_DIM;
                draw_input_row(history.entries.at(i), text_color);
            }
        }

        ImGui::PopFont();

        OverlayCanvas::end_panel();
    }
}
