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

#include "configuration_manager.hpp"

#include <fstream>

#include <toml++/toml.hpp> // NOLINT

#include "logging.h"
#include "platform_configuration.hpp"

namespace {

constexpr const char *DEFAULT_CONFIGURATION_FILE = "config.toml";
constexpr const char *UI_KEY = "ui";
constexpr const char *SHOW_COMMAND_HISTORY_KEY = "show_command_history";
constexpr const char *SHOW_PLAYER2_COMMAND_HISTORY_KEY = "show_player2_command_history";
constexpr const char *SHOW_OWN_FRAME_DATA_KEY = "show_own_frame_data";
constexpr const char *SHOW_OPPONENT_FRAME_DATA_KEY = "show_opponent_frame_data";

void load_ui(const toml::table &table, ConfigurationData::Ui &ui) {
    if (const toml::value<bool> *const value = table.get_as<bool>(SHOW_COMMAND_HISTORY_KEY)) {
        ui.show_command_history = value->get();
    }

    if (const toml::value<bool> *const value = table.get_as<bool>(SHOW_PLAYER2_COMMAND_HISTORY_KEY)) {
        ui.show_player2_command_history = value->get();
    }

    if (const toml::value<bool> *const value = table.get_as<bool>(SHOW_OWN_FRAME_DATA_KEY)) {
        ui.show_own_frame_data = value->get();
    }

    if (const toml::value<bool> *const value = table.get_as<bool>(SHOW_OPPONENT_FRAME_DATA_KEY)) {
        ui.show_opponent_frame_data = value->get();
    }
}

toml::table save_ui(const ConfigurationData::Ui &ui) {
    return toml::table{
        {SHOW_COMMAND_HISTORY_KEY, ui.show_command_history},
        {SHOW_PLAYER2_COMMAND_HISTORY_KEY, ui.show_player2_command_history},
        {SHOW_OWN_FRAME_DATA_KEY, ui.show_own_frame_data},
        {SHOW_OPPONENT_FRAME_DATA_KEY, ui.show_opponent_frame_data},
    };
}

} // namespace

bool ConfigurationManager::load(const std::filesystem::path &path, ConfigurationData &configuration) {
    try {
        const toml::table table = toml::parse_file(path.string());

        const toml::table *const ui = table.get_as<toml::table>(UI_KEY);
        if (ui != nullptr) {
            load_ui(*ui, configuration.ui);
        }

        return true;
    } catch (const toml::parse_error &) {
        return false;
    }
}

bool ConfigurationManager::load_default(ConfigurationData &configuration) {
    const std::filesystem::path path = default_path();
    if (path.empty()) {
        log_debug("failed to resolve executable directory");
        return false;
    }

    if (!std::filesystem::exists(path)) {
        log_debug("configuration file not found: %s", path.string().c_str());
        return true;
    }

    if (!load(path, configuration)) {
        log_warn("failed to load configuration file: %s", path.string().c_str());
        return false;
    }

    return true;
}

std::filesystem::path ConfigurationManager::default_path() {
    const std::filesystem::path directory = executable_directory();
    if (directory.empty()) {
        return {};
    }

    return directory / DEFAULT_CONFIGURATION_FILE;
}

bool ConfigurationManager::save(const std::filesystem::path &path, const ConfigurationData &configuration) {
    const toml::table table{{UI_KEY, save_ui(configuration.ui)}};

    std::ofstream stream(path);
    if (!stream) {
        return false;
    }

    stream << table;
    return static_cast<bool>(stream);
}
