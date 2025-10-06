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

#ifndef MEMORY_READER_H
#define MEMORY_READER_H

#include <stddef.h>
#include <stdint.h>

/**
 * Finds T6 process ID and initializes the memory
 *
 * @return MR_INIT value
 */
int platform_init_memory_reader(void);

int read_bytes_raw(const long long address, void *buf, const size_t size);
int read_4bytes(const long long address, int32_t *value);
int read_2bytes(const long long address, int16_t *value);

#endif
