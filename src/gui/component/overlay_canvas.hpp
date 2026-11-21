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

#ifndef OVERLAY_CANVAS_HPP
#define OVERLAY_CANVAS_HPP

#include <algorithm>
#include <cstdint>

#include <imgui.h> // NOLINT

#include "color_scheme.hpp"
#include "gui_constants.hpp"

namespace ui {

inline float em(const float value) {
    const ImGuiStyle &style = ImGui::GetStyle();
    return style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi * value;
}

inline ImVec2 em(const float x, const float y) {
    return {em(x), em(y)};
}

inline float hairline() {
    return std::max(1.0F, em(0.04F));
}

} // namespace ui

enum class HorizontalAnchor : uint8_t {
    LEFT,
    CENTER,
    RIGHT,
};

enum class VerticalAnchor : uint8_t {
    TOP,
    CENTER,
    BOTTOM,
};

struct OverlayPanel {
    const char *id = nullptr;
    ImVec2 size;
    ImVec2 position;
    float background_alpha = color_scheme::PANEL_BACKGROUND_ALPHA;
    float border_size = ui::hairline();
};

struct RelativePanel {
    const char *id = nullptr;
    HorizontalAnchor horizontal = HorizontalAnchor::LEFT;
    VerticalAnchor vertical = VerticalAnchor::TOP;
    ImVec2 offset;
    ImVec2 size;
    float background_alpha = color_scheme::PANEL_BACKGROUND_ALPHA;
    float border_size = ui::hairline();
};

class OverlayCanvas {
public:
    static ImVec2 size();
    static OverlayPanel resolve(const RelativePanel &panel, const ImVec2 canvas);
    static void begin_panel(const OverlayPanel &panel);
    static void begin_panel(const RelativePanel &panel, const ImVec2 canvas);
    static void end_panel();
};

#endif
