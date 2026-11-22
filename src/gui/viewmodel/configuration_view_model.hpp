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

#include <chrono>
#include <filesystem>
#include <string>

#include "configuration.hpp"
#include "events.hpp"

struct ConfigurationModel {
    bool open = false;
    bool show_hit_level = true;
    bool toast_active = false;
    std::chrono::steady_clock::time_point toast_expires_at;
    std::string toast_text;
};

class ConfigurationViewModel {
public:
    explicit ConfigurationViewModel(ConfigurationData *configuration) noexcept;

    void on_event(const KeyEvent &event);

    void set_open(bool value);
    [[nodiscard]] bool is_open() const;

    void set_disable_config_button(bool value);

    void set_save_path(std::filesystem::path path);
    bool save();
    bool toast_visible();
    [[nodiscard]] const std::string &toast_message() const;

    [[nodiscard]] ConfigurationData &configuration();
    [[nodiscard]] const ConfigurationData &configuration() const;

    void set_show_hit_level(bool value);
    [[nodiscard]] bool show_hit_level() const;

private:
    void toast(std::string message);

    ConfigurationData *m_configuration;
    std::filesystem::path m_save_path;
    ConfigurationModel m_model{};
};

#endif
