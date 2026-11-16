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

#include "input_dumper.h"

#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "memory_reader_types.h"

static FILE *g_input_dump = NULL;

static const struct InputDumpHeader input_dump_header = {
    .magic = {'T', '6', 'M', 'S'},
    .version = INPUT_DUMP_VERSION,
    .record_size = sizeof(struct InputDumpRecord),
};

int input_dump_start(const char *base_path) {
    if (g_input_dump != NULL) {
        log_error("input dump is already open");
        return READ_ERROR;
    }

    if (base_path == NULL || base_path[0] == '\0') {
        log_error("no input dump base path given");
        return READ_ERROR;
    }

    char path[INPUT_DUMP_PATH_MAX];
    const size_t base_len = strlen(base_path);
    const size_t extension_len = strlen(INPUT_DUMP_EXTENSION);
    bool has_extension = false;
    if (base_len >= extension_len) {
        has_extension = strcmp(base_path + base_len - extension_len, INPUT_DUMP_EXTENSION) == 0;
    }

    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    const int written = snprintf(path,
                                 sizeof(path),
                                 has_extension ? "%s" : "%s%s",
                                 base_path,
                                 has_extension ? "" : INPUT_DUMP_EXTENSION);
    if (written < 0 || written >= (int) sizeof(path)) {
        log_error("input dump path is too long");
        return READ_ERROR;
    }

    g_input_dump = fopen(path, "wb");
    if (g_input_dump == NULL) {
        log_error("failed to open input dump \"%s\"", path);
        return READ_ERROR;
    }

    if (fwrite(&input_dump_header, sizeof(input_dump_header), 1, g_input_dump) != 1) {
        log_error("failed to write input dump header");
        if (fclose(g_input_dump) != 0) {
            log_error("failed to close input dump");
        }
        g_input_dump = NULL;
        return READ_ERROR;
    }

    log_info("recording input dump to \"%s\"", path);

    return READ_OK;
}

int input_dump_write(const uint32_t game_frame, const uint32_t raw) {
    if (g_input_dump == NULL) {
        return READ_OK;
    }

    const struct InputDumpRecord record = {
        .game_frame = game_frame,
        .raw = raw,
    };

    if (fwrite(&record, sizeof(record), 1, g_input_dump) != 1 || fflush(g_input_dump) != 0) {
        log_error("failed to write input dump record");
        return READ_ERROR;
    }

    return READ_OK;
}

void input_dump_stop(void) {
    if (g_input_dump == NULL) {
        return;
    }

    if (fclose(g_input_dump) != 0) {
        log_error("failed to close input dump");
    }

    g_input_dump = NULL;
}
