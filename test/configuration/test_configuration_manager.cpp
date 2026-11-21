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

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "configuration_manager.hpp"

namespace {

constexpr const char *INVALID_CONFIGURATION = "invalid = [";

class TestConfigurationManager : public testing::Test {
protected:
    std::filesystem::path m_path;

    void SetUp() override {
        m_path = std::filesystem::path(testing::TempDir()) / "t6fdtool_test_config.toml";
        std::filesystem::remove(m_path);
    }

    void TearDown() override {
        std::filesystem::remove(m_path);
    }

    void write_file(const std::string &content) const {
        std::ofstream stream(m_path);
        stream << content;
    }
};

TEST_F(TestConfigurationManager, loads_ui_values) {
    write_file("[ui]\n"
               "show_command_history = false\n"
               "show_player2_command_history = false\n"
               "show_own_frame_data = false\n"
               "show_opponent_frame_data = false\n");

    ConfigurationData configuration;

    ASSERT_TRUE(ConfigurationManager::load(m_path, configuration));
    EXPECT_FALSE(configuration.ui.show_command_history);
    EXPECT_FALSE(configuration.ui.show_player2_command_history);
    EXPECT_FALSE(configuration.ui.show_own_frame_data);
    EXPECT_FALSE(configuration.ui.show_opponent_frame_data);
}

TEST_F(TestConfigurationManager, keeps_defaults_for_missing_fields) {
    write_file("[ui]\n"
               "show_command_history = false\n");

    ConfigurationData configuration;

    ASSERT_TRUE(ConfigurationManager::load(m_path, configuration));
    EXPECT_FALSE(configuration.ui.show_command_history);
    EXPECT_TRUE(configuration.ui.show_player2_command_history);
    EXPECT_TRUE(configuration.ui.show_own_frame_data);
    EXPECT_TRUE(configuration.ui.show_opponent_frame_data);
}

TEST_F(TestConfigurationManager, keeps_defaults_for_empty_file) {
    write_file("");

    ConfigurationData configuration;

    ASSERT_TRUE(ConfigurationManager::load(m_path, configuration));
    EXPECT_TRUE(configuration.ui.show_command_history);
    EXPECT_TRUE(configuration.ui.show_player2_command_history);
    EXPECT_TRUE(configuration.ui.show_own_frame_data);
    EXPECT_TRUE(configuration.ui.show_opponent_frame_data);
}

TEST_F(TestConfigurationManager, returns_false_for_missing_file) {
    ConfigurationData configuration;

    EXPECT_FALSE(ConfigurationManager::load(m_path, configuration));
}

TEST_F(TestConfigurationManager, returns_false_for_invalid_file) {
    write_file(INVALID_CONFIGURATION);

    ConfigurationData configuration;

    EXPECT_FALSE(ConfigurationManager::load(m_path, configuration));
}

TEST_F(TestConfigurationManager, saves_ui_values) {
    ConfigurationData configuration;
    configuration.ui.show_command_history = false;
    configuration.ui.show_player2_command_history = false;
    configuration.ui.show_own_frame_data = true;
    configuration.ui.show_opponent_frame_data = false;

    ASSERT_TRUE(ConfigurationManager::save(m_path, configuration));

    ConfigurationData loaded;
    ASSERT_TRUE(ConfigurationManager::load(m_path, loaded));
    EXPECT_FALSE(loaded.ui.show_command_history);
    EXPECT_FALSE(loaded.ui.show_player2_command_history);
    EXPECT_TRUE(loaded.ui.show_own_frame_data);
    EXPECT_FALSE(loaded.ui.show_opponent_frame_data);
}

} // namespace
