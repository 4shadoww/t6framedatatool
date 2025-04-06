#define _GNU_SOURCE
#include <stdio.h>

#include <sys/uio.h>

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
    set_read_address((void*) address);
    const size_t nread = process_vm_readv(g_pid, g_local, 1, g_remote, 1, 0);
    if (nread != 4) {
        log_error("failed to read memory: %zu", nread);
        return -1;
    }
    return big32_to_little(g_buf);

}

int init_memory_reader() {
    if (g_initialized) {
        return MR_INIT_ALREADY_DONE;
    }

    // TODO: find PID
    g_pid = 613894;

    if (g_pid == -1) {
        return MR_INIT_ERROR;
    }

    g_local[0].iov_base = g_buf;
    g_local[0].iov_len = READ_BUFFER_LEN;
    g_remote[0].iov_len = READ_BUFFER_LEN;

    g_initialized = 1;
    return MR_INIT_OK;
}

uint32_t p1_frames_last_action() {
    return read_4bytes(P1_FRAMES_LAST_ACTION);
}

uint32_t p1_frames_last_connection() {
    return read_4bytes(P1_FRAMES_LAST_CONNECTION);
}

uint32_t p1_recovery_frames() {
    return read_4bytes(P1_RECOVERY_FRAMES);
}

uint32_t p2_frames_last_action() {
    return read_4bytes(P2_FRAMES_LAST_ACTION);
}

uint32_t p2_frames_last_connection() {
    return read_4bytes(P2_FRAMES_LAST_CONNECTION);
}

uint32_t p2_recovery_frames() {
    return read_4bytes(P2_RECOVERY_FRAMES);
}

uint32_t current_game_frame() {
    return read_4bytes(CURRENT_GAME_FRAME);
}

int read_game_state(struct game_state *state) {
    uint32_t value = current_game_frame();
    if (value == -1) {
        log_debug("readed invalid game frame number");
        return -1;
    }
    state->game_frame = value;

    value = p1_frames_last_action();
    if (value == -1) {
        log_debug("readed invalid p1 last action frames");
        return -1;
    }
    state->p1_frames_last_action = value;

    value = p1_frames_last_connection();
    if (value == -1) {
        log_debug("readed invalid p1 last connection frames");
        return -1;
    }
    state->p1_frames_last_connection = value;

    value = p1_recovery_frames();
    if (value == -1) {
        log_debug("readed invalid p1 recovery frames");
        return -1;
    }
    state->p1_recovery_frames = value;

    value = p2_frames_last_action();
    if (value == -1) {
        log_debug("readed invalid p2 last action frames");
        return -1;
    }
    state->p2_frames_last_action = value;

    value = p2_frames_last_connection();
    if (value == -1) {
        log_debug("readed invalid p2 last connection frames");
        return -1;
    }
    state->p2_frames_last_connection = value;

    value = p2_recovery_frames();
    if (value == -1) {
        log_debug("readed invalid p2 recovery frames");
        return -1;
    }
    state->p2_recovery_frames = value;


    return 0;
}
