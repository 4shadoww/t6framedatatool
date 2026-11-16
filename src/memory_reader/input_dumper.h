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

#ifndef INPUT_DUMP_H
#define INPUT_DUMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define INPUT_DUMP_MAGIC "T6MS"
#define INPUT_DUMP_VERSION 1
#define INPUT_DUMP_EXTENSION ".t6ms"

struct InputDumpHeader {
    char magic[4];
    uint32_t version;
    uint32_t record_size;
};

struct InputDumpRecord {
    uint32_t game_frame;
    uint32_t raw;
};

/*
 * Start recording raw inputs next to a frame dump
 * @param base_path frame dump file path
 * @return 0 on success, -1 on error
 */
int input_dump_start(const char *base_path);

/*
 * Write one frame of raw input
 * @return 0 on success, -1 on error
 */
int input_dump_write(uint32_t game_frame, uint32_t raw);

/*
 * Finish the input dump
 */
void input_dump_stop(void);

#ifdef __cplusplus
};
#endif

#endif
