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

#include <string>

#include "input_key.hpp"

TEST(InputKey, default_open_config_key_is_f1) {
    EXPECT_EQ(keybinds::open_config_key(), Key::F1);
}

TEST(InputKey, names_match_enum) {
    for (std::size_t i = 0; i < KEY_COUNT; i++) {
        const auto key = static_cast<Key>(i);
        EXPECT_STREQ(key_name(key), KEY_NAMES.at(i));
    }
}

TEST(InputKey, parses_all_names) {
    for (std::size_t i = 0; i < KEY_COUNT; i++) {
        const auto parsed = key_from_name(KEY_NAMES.at(i));
        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(*parsed, static_cast<Key>(i));
    }
}

TEST(InputKey, parses_names_case_insensitively) {
    const auto lower = key_from_name("f1");
    ASSERT_TRUE(lower.has_value());
    EXPECT_EQ(*lower, Key::F1);

    const auto mixed = key_from_name("f12");
    ASSERT_TRUE(mixed.has_value());
    EXPECT_EQ(*mixed, Key::F12);
}

TEST(InputKey, rejects_unknown_names) {
    EXPECT_FALSE(key_from_name("").has_value());
    EXPECT_FALSE(key_from_name("F0").has_value());
    EXPECT_FALSE(key_from_name("F13").has_value());
    EXPECT_FALSE(key_from_name("Enter").has_value());
    EXPECT_FALSE(key_from_name("F1 ").has_value());
}
