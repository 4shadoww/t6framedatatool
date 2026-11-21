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

#include <imgui.h> // NOLINT

void UiConfigView::draw(ConfigurationViewModel &viewModel) {
    ConfigurationData &configuration = viewModel.configuration();

    ImGui::Checkbox("Show command history", &configuration.ui.show_command_history);
    ImGui::Checkbox("Show Player 2 command history", &configuration.ui.show_player2_command_history);
    ImGui::Checkbox("Show own frame data", &configuration.ui.show_own_frame_data);
    ImGui::Checkbox("Show opponent frame data", &configuration.ui.show_opponent_frame_data);
    ImGui::Checkbox("Show UI only in practice mode", &configuration.ui.show_only_in_practice);
}
