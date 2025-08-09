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

#ifndef NUMBER_CONVERSIONS_H
#define NUMBER_CONVERSIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int32_t big32_to_little(const char *const buf);
int16_t big16_to_little(const char *const buf);
uint64_t ps3_address_to_x64(const uint32_t address);
float big32_to_little_float(const char *const buf);

#ifdef __cplusplus
};
#endif

#endif
