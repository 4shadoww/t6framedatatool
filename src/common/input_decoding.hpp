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

#ifndef INPUT_DECODING_HPP
#define INPUT_DECODING_HPP

#include <cstdint>

#include "frame_data_defs.hpp"

namespace input_decoding {

constexpr uint32_t DIRECTION_SHIFT = 20U;
constexpr uint32_t DIRECTION_MASK = 0xFU;

constexpr uint32_t BUTTON_1_MASK = 0x8000U;
constexpr uint32_t BUTTON_2_MASK = 0x1000U;
constexpr uint32_t BUTTON_3_MASK = 0x4000U;
constexpr uint32_t BUTTON_4_MASK = 0x2000U;

constexpr uint32_t BUTTON_1 = 1U << 0U;
constexpr uint32_t BUTTON_2 = 1U << 1U;
constexpr uint32_t BUTTON_3 = 1U << 2U;
constexpr uint32_t BUTTON_4 = 1U << 3U;

constexpr uint32_t button(const uint32_t raw, const uint32_t mask, const uint32_t bit) {
    return (raw & mask) != 0U ? bit : 0U;
}

inline InputDirection direction(const uint32_t raw) {
    return static_cast<InputDirection>((raw >> DIRECTION_SHIFT) & DIRECTION_MASK);
}

inline uint8_t buttons(const uint32_t raw) {
    return static_cast<uint8_t>(button(raw, BUTTON_1_MASK, BUTTON_1) | button(raw, BUTTON_2_MASK, BUTTON_2) |
                                button(raw, BUTTON_3_MASK, BUTTON_3) | button(raw, BUTTON_4_MASK, BUTTON_4));
}

} // namespace input_decoding

#endif
