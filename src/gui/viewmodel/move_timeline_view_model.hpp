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

#ifndef MOVE_TIMELINE_VIEW_MODEL_HPP
#define MOVE_TIMELINE_VIEW_MODEL_HPP

#include <mutex>

#include "events.hpp"
#include "move_timeline.hpp"

struct MoveTimelineModel {
    MoveTimeline p1;
    MoveTimeline p2;
    uint32_t game_frame;
};

class MoveTimelineViewModel {
public:
    MoveTimelineViewModel() noexcept;

    void on_event(const StatusEvent &event);
    void on_event(const FrameDataEvent &event);
    void on_event(const FrameDataCorrectionEvent &event);

    [[nodiscard]] MoveTimelineModel snapshot() const;

private:
    void update_from_frame_data(const FrameDataPoint &data);

    MoveTimelineModel m_model{};
    mutable std::mutex m_mutex;
};

#endif
