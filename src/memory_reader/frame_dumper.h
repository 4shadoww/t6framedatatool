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

#ifndef FRAME_DUMP_H
#define FRAME_DUMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "game_state_reader.h"

#define FRAME_DUMP_MAGIC "T6FD"
#define FRAME_DUMP_VERSION 1

// Header of a recorded frame dump
struct FrameDumpHeader {
    char magic[4];
    uint32_t version;
    uint32_t record_size;
};

// One analysed frame with the player side used for flipping
struct FrameDumpRecord {
    int32_t player_side;
    struct GameFrame frame;
};

/*
 * Start recording analysed frames to a file
 * @param path output file path
 * @return 0 on success, -1 on error
 */
int frame_dump_start(const char *path);

/*
 * Write one frame to the open dump
 * @return 0 on success, -1 on error
 */
int frame_dump_write(const struct GameFrame *frame, const int32_t player_side);

/*
 * Finish the dump
 */
void frame_dump_stop(void);

#ifdef __cplusplus
};
#endif

#endif
