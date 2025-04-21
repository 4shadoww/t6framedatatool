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
#define READ_BUFFER_LEN 4
#define RPCS3_NAME "rpcs3"

// Reader initialized
static int g_initialized = 0;

// UIO variables
static pid_t g_pid = -1;
static char g_buf[READ_BUFFER_LEN];
static struct iovec g_local[1];
static struct iovec g_remote[1];

static inline void set_read_address(void *address) {
    g_remote[0].iov_base = address;
}

static inline uint32_t read_4bytes(const long address) {
    g_local[0].iov_len = READ_BUFFER_LEN;
    set_read_address((void*) address);

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != 4) {
        log_error("failed to read memory: %zu", nread);
        return -1;
    }
    return big32_to_little(g_buf);
}

static inline uint32_t read_2bytes(const long address) {
    g_local[0].iov_len = 2;
    set_read_address((void*) address);

    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != 2) {
        log_error("failed to read memory: %zu", nread);
        return -1;
    }
    return big16_to_little(g_buf);

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
    if (g_initialized) {
        return MR_INIT_ALREADY_DONE;
    }

    g_pid =  get_pid(RPCS3_NAME);

    if (g_pid == -1) {
        return MR_INIT_ERROR;
    }

    g_local[0].iov_base = g_buf;
    g_remote[0].iov_len = READ_BUFFER_LEN;

    g_initialized = 1;
    return MR_INIT_OK;
}

uint32_t p1_frames_last_action(void) {
    return read_4bytes(P1_FRAMES_LAST_ACTION);
}

uint32_t p1_connection(void) {
    return read_2bytes(P1_CONNECTION_BOOL);
}

uint32_t p1_recovery_frames(void) {
    return read_4bytes(P1_RECOVERY_FRAMES);
}

uint32_t p1_intent(void) {
    return read_4bytes(P1_INTENT);
}

uint32_t p2_frames_last_action(void) {
    return read_4bytes(P2_FRAMES_LAST_ACTION);
}

uint32_t p2_connection(void) {
    return read_2bytes(P2_CONNECTION_BOOL);
}

uint32_t p2_recovery_frames(void) {
    return read_4bytes(P2_RECOVERY_FRAMES);
}

uint32_t p2_intent(void) {
    return read_4bytes(P2_INTENT);
}

uint32_t current_game_frame(void) {
    return read_4bytes(CURRENT_GAME_FRAME);
}

int read_game_state(struct game_state *state) {
    uint32_t value = current_game_frame();

    if (value == READ_ERROR) {
        log_debug("readed invalid game frame number");
        return -1;
    }
    state->game_frame = value;

    value = p1_frames_last_action();
    if (value == READ_ERROR) {
        log_debug("readed invalid p1 last action frames");
        return -1;
    }
    state->p1_frames_last_action = value;

    value = p1_connection();
    if (value == READ_ERROR) {
        log_debug("readed invalid p1 last connection frames");
        return -1;
    }
    state->p1_connection = value;

    value = p1_recovery_frames();
    if (value == READ_ERROR) {
        log_debug("readed invalid p1 recovery frames");
        return -1;
    }
    state->p1_recovery_frames = value;

    value = p1_intent();
    if (value == READ_ERROR) {
        log_debug("readed invalid p1 intent");
        return -1;
    }
    state->p1_intent = value;

    value = p2_frames_last_action();
    if (value == READ_ERROR) {
        log_debug("readed invalid p2 last action frames");
        return -1;
    }
    state->p2_frames_last_action = value;

    value = p2_connection();
    if (value == READ_ERROR) {
        log_debug("readed invalid p2 last connection frames");
        return -1;
    }
    state->p2_connection = value;

    value = p2_recovery_frames();
    if (value == READ_ERROR) {
        log_debug("readed invalid p2 recovery frames");
        return -1;
    }
    state->p2_recovery_frames = value;

    value = p2_intent();
    if (value == READ_ERROR) {
        log_debug("readed invalid p2 intent");
        return -1;
    }
    state->p2_intent = value;

    return 0;
}
