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

#include <windows.h>

#include <tlhelp32.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "memory_reader.h"
#include "memory_reader_types.h"
#include "number_conversions.h"


/*
 * NOTE: warning this code for the major part is vibe coded
 * simply zero shits given to this port
 *
 * Memory reader for Windows platforms
 * NOT thread-safe
 */

// Constants
#define READ_BUFFER_LEN 12
#define RPCS3_NAME "rpcs3.exe"

// Win32 API variables
static HANDLE g_h_process = NULL; // Process handle for memory reading
static char g_buf[READ_BUFFER_LEN];

static DWORD get_pid_by_name(const char *processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    DWORD pid = 0;

    // Create a snapshot of all running processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        log_error("Failed to create toolhelp snapshot.");
        return 0;
    }

    // Find the process in the snapshot
    if (Process32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, processName) == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        }
        while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return pid;
}

int read_bytes_raw(const long long address, void *buf, const size_t size) {
    SIZE_T bytes_read = 0;
    // Use ReadProcessMemory instead of process_vm_readv
    if (ReadProcessMemory(g_h_process, (LPCVOID) address, buf, size, &bytes_read) == 0) { // NOLINT
        log_error("ReadProcessMemory failed with error code: %lu", GetLastError());
        return -1;
    }

    if (bytes_read != size) {
        log_error("failed to read %zu bytes (%zu)", size, bytes_read);
        return -1;
    }

    return (int) bytes_read;
}

int read_4bytes(const long long address, int32_t *value) {
    // Read directly into a temporary buffer
    if (read_bytes_raw(address, g_buf, 4) == -1) {
        log_error("failed to read 4 bytes");
        return READ_ERROR;
    }
    *value = big32_to_little(g_buf);
    return READ_OK;
}

int read_2bytes(const long long address, int16_t *value) {
    if (read_bytes_raw(address, g_buf, 2) == -1) {
        log_error("failed to read 2 bytes");
        return READ_ERROR;
    }
    *value = big16_to_little(g_buf);
    return READ_OK;
}

int platform_init_memory_reader(void) {
    DWORD pid = get_pid_by_name(RPCS3_NAME);

    if (pid == 0) {
        log_error("cannot find the emulator process: %s", RPCS3_NAME);
        return MR_INIT_ERROR;
    }

    // Open the process with read access rights
    g_h_process = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (g_h_process == NULL) {
        log_error("Failed to open process with PID %lu. Error code: %lu", pid, GetLastError());
        log_error("Try running this program as an administrator.");
        return MR_INIT_ERROR;
    }

    log_info("Successfully attached to process %s (PID: %lu)", RPCS3_NAME, pid);
    return MR_INIT_OK;
}

// --- Helper function to close the handle on exit ---
void cleanup_memory_reader(void) {
    if (g_h_process != NULL) {
        CloseHandle(g_h_process);
        g_h_process = NULL;
    }
}
