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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/uio.h>

#include "logging.h"
#include "number_conversions.h"

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
    set_read_address((void *) address); // NOLINT

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != size) {
        log_error("failed to read %zu bytes (%zu)", size, nread);
        return -1;
    }

    memcpy(buf, g_buf, size); // NOLINT
    return (int) nread;
}

static inline int read_4bytes(const long address, int32_t *value) {
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

static inline int read_2bytes(const long address, int16_t *value) {
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

pid_t get_pid(char *name) {
    static const char *directory = "/proc";

    DIR *dir = opendir(directory);

    if (dir == NULL) {
        return -1;
    }

    struct dirent *de = 0;

    while ((de = readdir(dir)) != 0) { // NOLINT: no thread safe warning
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        pid_t pid = -1;
        int res = sscanf(de->d_name, "%d", &pid); // NOLINT: warning nagging about using "strtol" as scanff iS NoT sAfE

        // PID valid
        if (res != 1) {
            continue;
        }

        // Check the process name from cmdline
        char cmdline_file[1024] = {0};
        sprintf(cmdline_file, "%s/%d/cmdline", directory, pid); // NOLINT

        FILE *cmdline = fopen(cmdline_file, "r");

        size_t bytes_read = 0;
        char *process_name = NULL;

        if (getline(&process_name, &bytes_read, cmdline) <= 0) { // NOLINT
            (void) fclose(cmdline); // NOLINT
            continue;
        }

        if (strstr(process_name, name) == 0) {
            (void) fclose(cmdline);
            continue;
        }

        // Found PID close streams
        (void) fclose(cmdline);
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

    g_local[0].iov_base = g_buf;
    g_remote[0].iov_len = READ_BUFFER_LEN;

    return MR_INIT_OK;
}

