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

#include "frame_dumper.h"

#include <stdio.h>

#include "logging.h"
#include "memory_reader_types.h"

static FILE *g_frame_dump = NULL;

static const struct FrameDumpHeader frame_dump_header = {
    .magic = {'T', '6', 'F', 'D'},
    .version = FRAME_DUMP_VERSION,
    .record_size = sizeof(struct FrameDumpRecord),
};

int frame_dump_start(const char *path) {
    if (g_frame_dump != NULL) {
        log_error("frame dump is already open");
        return READ_ERROR;
    }

    if (path == NULL || path[0] == '\0') {
        log_error("no frame dump path given");
        return READ_ERROR;
    }

    g_frame_dump = fopen(path, "wb");
    if (g_frame_dump == NULL) {
        log_error("failed to open frame dump \"%s\"", path);
        return READ_ERROR;
    }

    if (fwrite(&frame_dump_header, sizeof(frame_dump_header), 1, g_frame_dump) != 1) {
        log_error("failed to write frame dump header");
        if (fclose(g_frame_dump) != 0) {
            log_error("failed to close frame dump");
        }
        g_frame_dump = NULL;
        return READ_ERROR;
    }

    log_info("recording frame dump to \"%s\"", path);

    return READ_OK;
}

int frame_dump_write(const struct GameFrame *frame, const int32_t player_side) {
    if (g_frame_dump == NULL) {
        return READ_OK;
    }

    if (frame == NULL) {
        log_error("frame dump record has no frame");
        return READ_ERROR;
    }

    const struct FrameDumpRecord record = {
        .player_side = player_side,
        .frame = *frame,
    };

    if (fwrite(&record, sizeof(record), 1, g_frame_dump) != 1 || fflush(g_frame_dump) != 0) {
        log_error("failed to write frame dump record");
        return READ_ERROR;
    }

    return READ_OK;
}

void frame_dump_stop(void) {
    if (g_frame_dump == NULL) {
        return;
    }

    if (fclose(g_frame_dump) != 0) {
        log_error("failed to close frame dump");
    }

    g_frame_dump = NULL;
}
