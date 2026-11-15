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

#ifndef EVENT_BUS_HPP
#define EVENT_BUS_HPP

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <vector>

class EventBus {
public:
    template<typename EVENT>
    static void dispatch(const EVENT &event) {
        const std::shared_lock lock(mutex<EVENT>());
        for (const auto &handler : handlers<EVENT>()) {
            handler(event);
        }
    }

    template<typename EVENT, typename RECEIVER>
    static void subscribe(RECEIVER *receiver) {
        const std::unique_lock lock(mutex<EVENT>());
        handlers<EVENT>().push_back([receiver](const EVENT &event) { receiver->on_event(event); });
    }

    template<typename EVENT>
    static void clear() {
        const std::unique_lock lock(mutex<EVENT>());
        handlers<EVENT>().clear();
    }

private:
    template<typename EVENT>
    static std::vector<std::function<void(const EVENT &)>> &handlers() {
        static std::vector<std::function<void(const EVENT &)>> list;
        return list;
    }

    template<typename EVENT>
    static std::shared_mutex &mutex() {
        static std::shared_mutex m;
        return m;
    }
};

#endif
