/*
  Copyright (C) 2025 Noa-Emil Nissinen

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

#include <stdint.h>

#include "number_conversions.h"

int32_t big32_to_little(const char *const buf) {
    union {
        char bytes[4];
        int32_t value;
    } u;

    u.bytes[0] = buf[3];
    u.bytes[1] = buf[2];
    u.bytes[2] = buf[1];
    u.bytes[3] = buf[0];

    return u.value;
}

int16_t big16_to_little(const char *const buf) {
    union {
        char bytes[2];
        int16_t value;
    } u;

    u.bytes[0] = buf[1];
    u.bytes[1] = buf[0];

    return u.value;
}

uint64_t ps3_address_to_x64(const uint32_t address) {
    static const uint64_t BASE_ADDRESS = 0x300000000;
    static const uint8_t OFFSET = 104;

    return address + BASE_ADDRESS + OFFSET;
}

float big32_to_little_float(const char *const buf) {
    union {
        char bytes[4];
        float value;
    } u;

    u.bytes[0] = buf[3];
    u.bytes[1] = buf[2];
    u.bytes[2] = buf[1];
    u.bytes[3] = buf[0];

    return u.value;
}
