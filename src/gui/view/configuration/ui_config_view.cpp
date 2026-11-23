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

#include "view/configuration/ui_config_view.hpp"

#include <cmath>

#include <imgui.h> // NOLINT

namespace {

constexpr float PERCENT_FACTOR = 100.0F;

} // namespace

void UiConfigView::draw(ConfigurationViewModel &viewModel) {
    ConfigurationData &configuration = viewModel.configuration();

    ImGui::Checkbox("Show command history", &configuration.ui.show_command_history);
    ImGui::Checkbox("Show Player 2 command history", &configuration.ui.show_player2_command_history);
    ImGui::Checkbox("Show own frame data", &configuration.ui.show_own_frame_data);
    ImGui::Checkbox("Show opponent frame data", &configuration.ui.show_opponent_frame_data);
    ImGui::Checkbox("Show UI only in practice mode", &configuration.ui.show_only_in_practice);
    ImGui::Checkbox("Show move timeline", &configuration.ui.show_move_timeline);
    bool disable_config_button = configuration.ui.disable_config_button;
    if (ImGui::Checkbox("Disable configuration button", &disable_config_button)) {
        viewModel.set_disable_config_button(disable_config_button);
    }

    int scale_percent = static_cast<int>(std::lround(configuration.ui.scale * PERCENT_FACTOR));
    const int min_percent = static_cast<int>(MIN_UI_SCALE * PERCENT_FACTOR);
    const int max_percent = static_cast<int>(MAX_UI_SCALE * PERCENT_FACTOR);
    if (ImGui::SliderInt("UI scale", &scale_percent, min_percent, max_percent, "%d%%")) {
        configuration.ui.scale = static_cast<float>(scale_percent) / PERCENT_FACTOR;
    }
}
