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

#include <chrono>
#include <utility>

#include "configuration_manager.hpp"

namespace {

constexpr auto SAVE_MESSAGE_DURATION = std::chrono::seconds(2);

} // namespace

ConfigurationViewModel::ConfigurationViewModel(ConfigurationData *configuration) noexcept
    : m_configuration(configuration) {}

void ConfigurationViewModel::set_open(const bool value) {
    m_model.open = value;
}

bool ConfigurationViewModel::is_open() const {
    return m_model.open;
}

void ConfigurationViewModel::set_save_path(std::filesystem::path path) {
    m_save_path = std::move(path);
}

bool ConfigurationViewModel::save() {
    if (m_save_path.empty()) {
        return false;
    }

    const bool saved = ConfigurationManager::save(m_save_path, *m_configuration);
    if (saved) {
        m_model.save_message_active = true;
        m_model.save_message_expires_at = std::chrono::steady_clock::now() + SAVE_MESSAGE_DURATION;
    }

    return saved;
}

bool ConfigurationViewModel::show_saved_message() {
    if (!m_model.save_message_active) {
        return false;
    }

    if (std::chrono::steady_clock::now() >= m_model.save_message_expires_at) {
        m_model.save_message_active = false;
        return false;
    }

    return true;
}

ConfigurationData &ConfigurationViewModel::configuration() {
    return *m_configuration;
}

const ConfigurationData &ConfigurationViewModel::configuration() const {
    return *m_configuration;
}

void ConfigurationViewModel::set_show_hit_level(const bool value) {
    m_model.show_hit_level = value;
}

bool ConfigurationViewModel::show_hit_level() const {
    return m_model.show_hit_level;
}
