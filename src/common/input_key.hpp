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

#ifndef INPUT_KEY_HPP
#define INPUT_KEY_HPP

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

enum class Key : std::uint8_t {
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
};

inline constexpr std::size_t KEY_COUNT = 12;

inline constexpr std::array<const char *, KEY_COUNT> KEY_NAMES = {
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
};

constexpr std::size_t key_index(const Key key) {
    return static_cast<std::size_t>(key);
}

inline const char *key_name(const Key key) {
    return KEY_NAMES.at(key_index(key));
}

inline std::optional<Key> key_from_name(const std::string_view name) {
    for (std::size_t i = 0; i < KEY_COUNT; i++) {
        const std::string_view candidate = KEY_NAMES.at(i);
        if (candidate.size() != name.size()) {
            continue;
        }

        bool matches = true;
        for (std::size_t j = 0; j < name.size(); j++) {
            const auto lhs = static_cast<unsigned char>(candidate.at(j));
            const auto rhs = static_cast<unsigned char>(name.at(j));
            if (std::toupper(lhs) != std::toupper(rhs)) {
                matches = false;
                break;
            }
        }

        if (matches) {
            return static_cast<Key>(i);
        }
    }

    return std::nullopt;
}

namespace keybinds {
constexpr Key open_config_key() {
    return Key::F1;
}
} // namespace keybinds

#endif
