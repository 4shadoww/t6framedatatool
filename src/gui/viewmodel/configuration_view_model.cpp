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

#include "viewmodel/configuration_view_model.hpp"

void ConfigurationViewModel::set_open(const bool value) {
    m_model.open = value;
}

bool ConfigurationViewModel::is_open() const {
    return m_model.open;
}

void ConfigurationViewModel::set_show_frame_data(const bool value) {
    m_model.show_frame_data = value;
}

bool ConfigurationViewModel::show_frame_data() const {
    return m_model.show_frame_data;
}

void ConfigurationViewModel::set_show_command_history(const bool value) {
    m_model.show_command_history = value;
}

bool ConfigurationViewModel::show_command_history() const {
    return m_model.show_command_history;
}

void ConfigurationViewModel::set_show_hit_level(const bool value) {
    m_model.show_hit_level = value;
}

bool ConfigurationViewModel::show_hit_level() const {
    return m_model.show_hit_level;
}
