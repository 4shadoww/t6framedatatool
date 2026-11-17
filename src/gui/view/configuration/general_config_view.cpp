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

#include "view/configuration/general_config_view.hpp"

#include <imgui.h> // NOLINT

void GeneralConfigView::draw(ConfigurationViewModel &viewModel) {
    bool show_frame_data = viewModel.show_frame_data();
    if (ImGui::Checkbox("Show frame data", &show_frame_data)) {
        viewModel.set_show_frame_data(show_frame_data);
    }

    bool show_command_history = viewModel.show_command_history();
    if (ImGui::Checkbox("Show command history", &show_command_history)) {
        viewModel.set_show_command_history(show_command_history);
    }
}
