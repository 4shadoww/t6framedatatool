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

#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include <sys/uio.h>
#include <sys/types.h>

#include "number_conversions.h"
#include "logging.h"

#include "address_config.h"
#include "memory_reader.h"

/*
 * Memory reader for linux platforms
 * NOT thread-safe
 */

// Constants
#define READ_BUFFER_LEN 12
#define RPCS3_NAME "rpcs3"

// UIO variables
static pid_t g_pid = -1;
static char g_buf[READ_BUFFER_LEN];
static struct iovec g_local[1];
static struct iovec g_remote[1];

static inline void set_read_address(void *address) {
    g_remote[0].iov_base = address;
}

static inline int read_bytes_raw(const long address, void *buf, const size_t size) {
    g_local[0].iov_len = size;
    set_read_address((void*) address);

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != size) {
        log_error("failed to read %zu bytes (%zu)", size, nread);
        return -1;
    }

    memcpy(buf, g_buf, size);
    return nread;
}

static inline int read_4bytes(const long address, int32_t *value) {
    g_local[0].iov_len = 4;
    set_read_address((void*) address);

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != 4) {
        log_error("failed to read 4 bytes (%zu)", nread);
        return READ_ERROR;
    }
    *value = big32_to_little(g_buf);
    return READ_OK;
}

static inline int read_2bytes(const long address, int16_t *value) {
    g_local[0].iov_len = 2;
    set_read_address((void*) address);

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != 2) {
        log_error("failed to read 2 bytes (%zu)", nread);
        return READ_ERROR;
    }
    *value = big16_to_little(g_buf);
    return READ_OK;

}

pid_t get_pid(char *name) {
    static const char* directory = "/proc";

    DIR* dir = opendir(directory);

    if (dir == NULL) {
        return -1;
    }

    struct dirent* de = 0;

    while ((de = readdir(dir)) != 0) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        pid_t pid = -1;
        int res = sscanf(de->d_name, "%d", &pid);

        // PID valid
        if (res != 1) {
            continue;
        }

        // Check the process name from cmdline
        char cmdline_file[1024] = {0};
        sprintf(cmdline_file, "%s/%d/cmdline", directory, pid);

        FILE* cmdline = fopen(cmdline_file, "r");

        size_t bytes_read;
        char* process_name = NULL;

        if (getline(&process_name, &bytes_read, cmdline) <= 0) {
            fclose(cmdline);
            continue;
        }

        if (strstr(process_name, name) == 0) {
            fclose(cmdline);
            continue;
        }

        // Found PID close streams
        fclose(cmdline);
        closedir(dir);

        return pid;
    }

    closedir(dir);

    return -1;
}

int init_memory_reader(void) {
    g_pid = get_pid(RPCS3_NAME);

    if (g_pid == -1) {
        log_error("cannot find the emulator process");
        return MR_INIT_ERROR;
    }

    g_local[0].iov_base = g_buf;
    g_remote[0].iov_len = READ_BUFFER_LEN;

    return MR_INIT_OK;
}

int p1_frames_last_action(int32_t *value) {
    return read_4bytes(P1_FRAMES_LAST_ACTION, value);
}

int p1_connection(int16_t *value) {
    return read_2bytes(P1_CONNECTION_BOOL, value);
}

int p1_recovery_frames(uint32_t *value) {
    return read_4bytes(P1_RECOVERY_FRAMES, (int32_t*) value);
}

int32_t p1_intent(int32_t *value) {
    return read_4bytes(P1_INTENT, value);
}

int32_t p1_state(int32_t *value) {
    return read_4bytes(P1_STATE,value);
}

int p1_position(struct player_coordinate *value) {
    int read = read_bytes_raw(P1_POSITION, value, sizeof(struct player_coordinate));
    if (read == READ_ERROR) {
        return READ_ERROR;
    }

    value->x = big32_to_little_float((char*) value);
    value->y = big32_to_little_float(&((char*) value)[4]);
    value->z = big32_to_little_float(&((char*) value)[8]);

    return READ_OK;
}

int p2_frames_last_action(int32_t *value) {
    return read_4bytes(P2_FRAMES_LAST_ACTION, value);
}

int p2_connection(int16_t *value) {
    return read_2bytes(P2_CONNECTION_BOOL, value);
}

int p2_recovery_frames(uint32_t *value) {
    return read_4bytes(P2_RECOVERY_FRAMES, (int32_t*) value);
}

int p2_intent(int32_t *value) {
    return read_4bytes(P2_INTENT, value);
}

int p2_state(int32_t *value) {
    return read_4bytes(P2_STATE, value);
}

int p2_position(struct player_coordinate *value) {
    int read = read_bytes_raw(P2_POSITION, value, sizeof(struct player_coordinate));
    if (read == READ_ERROR) {
        return READ_ERROR;
    }

    value->x = big32_to_little_float((char*) value);
    value->y = big32_to_little_float(&((char*) value)[4]);
    value->z = big32_to_little_float(&((char*) value)[8]);

    return READ_OK;
}

int current_game_frame(uint32_t *value) {
    return read_4bytes(CURRENT_GAME_FRAME, (int32_t*) value);
}

int player_side(int32_t *value) {
    return read_4bytes(PLAYER_SIDE, value);
}

int read_game_state(struct game_state *state) {
    int32_t value;
    struct player_coordinate coords = {0};

    int result = current_game_frame((uint32_t*) &value);

    if (result == READ_ERROR) {
        log_debug("readed invalid game frame number");
        return READ_ERROR;
    }
    state->game_frame = (uint32_t) value;

    result = p1_frames_last_action(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 last action frames");
        return READ_ERROR;
    }
    state->p1_frames_last_action = value;

    result = p1_connection((int16_t*) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 last connection frames");
        return READ_ERROR;
    }
    state->p1_connection = (int16_t) value;

    result = p1_recovery_frames((uint32_t*) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 recovery frames");
        return READ_ERROR;
    }
    state->p1_recovery_frames = (uint32_t) value;

    result = p1_intent(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 intent");
        return READ_ERROR;
    }
    state->p1_intent = value;

    result = p1_state(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p1_state = value;


    result = p1_position(&coords);
    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p1_position = coords;

    result = p2_frames_last_action(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 last action frames");
        return READ_ERROR;
    }
    state->p2_frames_last_action = value;

    result = p2_connection((int16_t*) &value);
    if (value == READ_ERROR) {
        log_debug("readed invalid p2 last connection frames");
        return READ_ERROR;
    }
    state->p2_connection = (int16_t) value;

    result = p2_recovery_frames((uint32_t*) &value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 recovery frames");
        return READ_ERROR;
    }
    state->p2_recovery_frames = value;

    result = p2_intent(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 intent");
        return READ_ERROR;
    }
    state->p2_intent = value;

    result = p2_state(&value);
    if (result == READ_ERROR) {
        log_debug("readed invalid p2 state");
        return READ_ERROR;
    }
    state->p2_state = value;

    memset(&coords, 0, sizeof(coords));

    result = p2_position(&coords);

    if (result == READ_ERROR) {
        log_debug("readed invalid p1 state");
        return READ_ERROR;
    }
    state->p2_position = coords;

    return 0;
}
