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

#ifndef CONFIGURATION_VIEW_MODEL_HPP
#define CONFIGURATION_VIEW_MODEL_HPP

struct ConfigurationModel {
    bool open = false;
    bool show_frame_data = true;
    bool show_command_history = true;
    bool show_hit_level = true;
};

class ConfigurationViewModel {
public:
    void set_open(bool value);
    [[nodiscard]] bool is_open() const;

    void set_show_frame_data(bool value);
    [[nodiscard]] bool show_frame_data() const;

    void set_show_command_history(bool value);
    [[nodiscard]] bool show_command_history() const;

    void set_show_hit_level(bool value);
    [[nodiscard]] bool show_hit_level() const;

private:
    ConfigurationModel m_model{};
};

#endif
