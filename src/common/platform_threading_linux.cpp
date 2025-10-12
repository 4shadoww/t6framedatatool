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
#include <sched.h>

#include "logging.h"

bool set_realtime_prio(std::thread &thread) {
    const int policy = SCHED_FIFO;
    struct sched_param param{};
    param.sched_priority = sched_get_priority_max(policy);

    const pthread_t native_handle = thread.native_handle();

    const int ret = pthread_setschedparam(native_handle, policy, &param);
    if (ret == 0) {
        log_debug("thread prio set to realtime");
        return true;
    } else {
        log_error("failed to set thread realtime prio");
        return false;
    }
}
