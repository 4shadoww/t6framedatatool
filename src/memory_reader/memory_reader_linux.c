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

#define _GNU_SOURCE // NOLINT
#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/uio.h>

#include "logging.h"
#include "number_conversions.h"

#include "memory_reader.h"
#include "memory_reader_types.h"

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

int read_bytes_raw(const long long address, void *buf, const size_t size) {
    g_local[0].iov_len = size;
    set_read_address((void *) address); // NOLINT

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != size) {
        log_error("failed to read %zu bytes (%zu)", size, nread);
        return -1;
    }

    memcpy(buf, g_buf, size); // NOLINT
    return (int) nread;
}

int read_4bytes(const long long address, int32_t *value) {
    g_local[0].iov_len = 4;
    set_read_address((void *) address); // NOLINT

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != 4) {
        log_error("failed to read 4 bytes (%zu)", nread);
        return READ_ERROR;
    }
    *value = big32_to_little(g_buf);
    return READ_OK;
}

int read_2bytes(const long long address, int16_t *value) {
    g_local[0].iov_len = 2;
    set_read_address((void *) address); // NOLINT

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != 2) {
        log_error("failed to read 2 bytes (%zu)", nread);
        return READ_ERROR;
    }
    *value = big16_to_little(g_buf);
    return READ_OK;
}

static int process_exe_basename_is(const pid_t pid, const char *name) {
    char link_path[64] = {0};
    char exe_path[PATH_MAX] = {0};

    const int path_len = snprintf(link_path, sizeof(link_path), "/proc/%d/exe", (int) pid);
    if (path_len < 0 || (size_t) path_len >= sizeof(link_path)) {
        return 0;
    }

    const ssize_t exe_len = readlink(link_path, exe_path, sizeof(exe_path) - 1);
    if (exe_len < 0 || (size_t) exe_len >= sizeof(exe_path) - 1) {
        return 0;
    }
    exe_path[exe_len] = '\0';

    const char *basename = exe_path;
    for (ssize_t i = 0; i < exe_len; i++) {
        if (exe_path[i] == '/') {
            basename = &exe_path[i + 1];
        }
    }

    return strcasecmp(basename, name) == 0;
}

pid_t get_pid(const char *name) {
    DIR *dir = opendir("/proc");

    if (dir == NULL) {
        return -1;
    }

    // Match /proc/<pid>/exe. An AppImage's cmdline contains "rpcs3", but that
    // process is the runtime stub. The emulator itself is AppRun.wrapped and
    // its executable basename is still "rpcs3".
    struct dirent *de = NULL;
    while ((de = readdir(dir)) != NULL) { // NOLINT: no thread safe warning
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        pid_t pid = -1;
        const int res = sscanf(de->d_name, "%d", &pid); // NOLINT: warning nagging about using "strtol"

        if (res != 1) {
            continue;
        }

        if (process_exe_basename_is(pid, name) == 0) {
            continue;
        }

        closedir(dir);
        return pid;
    }

    closedir(dir);

    return -1;
}

int platform_init_memory_reader(void) {
    g_pid = get_pid(RPCS3_NAME);

    if (g_pid == -1) {
        log_error("cannot find the emulator process");
        return MR_INIT_ERROR;
    }

    log_info("attached to emulator process %d", (int) g_pid);

    g_local[0].iov_base = g_buf;
    g_remote[0].iov_len = READ_BUFFER_LEN;

    return MR_INIT_OK;
}

