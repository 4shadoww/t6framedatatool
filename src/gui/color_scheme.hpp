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

#ifndef COLOR_SCHEME_HPP
#define COLOR_SCHEME_HPP

#include <array>

#include <imgui.h> // NOLINT

namespace color_scheme {

constexpr float PANEL_BACKGROUND_ALPHA = 0.92F;

constexpr ImVec4 TEXT = {0.92F, 0.92F, 0.92F, 1.0F};
constexpr ImVec4 MUTED_TEXT = {0.65F, 0.65F, 0.65F, 1.0F};
constexpr ImVec4 WINDOW_BACKGROUND = {0.02F, 0.02F, 0.02F, PANEL_BACKGROUND_ALPHA};
constexpr ImVec4 CHILD_BACKGROUND = {0.04F, 0.04F, 0.04F, 0.95F};
constexpr ImVec4 FRAME_BACKGROUND = {0.08F, 0.08F, 0.08F, 1.0F};
constexpr ImVec4 BORDER = {0.16F, 0.16F, 0.16F, 1.0F};
constexpr ImVec4 SEPARATOR = {0.16F, 0.16F, 0.16F, 1.0F};

constexpr ImVec4 BUTTON = WINDOW_BACKGROUND;
constexpr ImVec4 BUTTON_HOVER = {0.15F, 0.15F, 0.15F, 1.0F};
constexpr ImVec4 BUTTON_ACTIVE = {1.00F, 1.00F, 1.00F, 1.0F};

constexpr ImVec4 POSITIVE = {0.35F, 1.0F, 0.55F, 1.0F};
constexpr ImVec4 NEGATIVE = {1.0F, 0.35F, 0.35F, 1.0F};
constexpr ImVec4 CORRECTION_MARKER = {0.55F, 0.55F, 0.55F, 1.0F};

constexpr ImVec4 HIT_LEVEL_HIGH = {1.0F, 0.30F, 0.30F, 1.0F};
constexpr ImVec4 HIT_LEVEL_MID = {1.0F, 0.85F, 0.25F, 1.0F};
constexpr ImVec4 HIT_LEVEL_LOW = {0.35F, 0.65F, 1.0F, 1.0F};
constexpr ImVec4 HIT_LEVEL_SPECIAL_MID = {0.35F, 0.90F, 0.90F, 1.0F};
constexpr ImVec4 HIT_LEVEL_UNKNOWN = {0.70F, 0.70F, 0.70F, 1.0F};

constexpr ImU32 OUTLINE = IM_COL32(0, 0, 0, 255);
constexpr ImU32 BUTTON_INACTIVE_OUTLINE = IM_COL32(0, 0, 0, 80);
constexpr ImU32 BUTTON_INACTIVE = IM_COL32(170, 170, 170, 180);
constexpr ImU32 TEXT_BRIGHT = IM_COL32(255, 255, 255, 255);
constexpr ImU32 TEXT_DIM = IM_COL32(200, 200, 200, 220);

constexpr std::array<ImU32, 4> INPUT_BUTTON_COLORS = {
    IM_COL32(70, 140, 255, 255),
    IM_COL32(255, 205, 60, 255),
    IM_COL32(80, 210, 110, 255),
    IM_COL32(240, 80, 80, 255),
};

} // namespace color_scheme

#endif
