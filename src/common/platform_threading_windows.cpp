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

#include "platform_threading.hpp"

#include <pthread.h>
#include <windows.h>
#include <winnt.h>

#include "logging.h"

bool set_realtime_prio(std::thread &thread) {
    const pthread_t native_pthread_handle = thread.native_handle();

    HANDLE native_windows_handle = pthread_gethandle(native_pthread_handle);

    if (SetThreadPriority(native_windows_handle, THREAD_PRIORITY_TIME_CRITICAL) != 0) {
        log_debug("thread prio set to realtime");
        return true;
    } else {
        log_error("failed to set thread realtime prio");
        return false;
    }
}
