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

#include "viewmodel/move_timeline_view_model.hpp"

#include <algorithm>

#include "event_bus.hpp"

MoveTimelineViewModel::MoveTimelineViewModel() noexcept {
    EventBus::subscribe<StatusEvent>(this);
    EventBus::subscribe<FrameDataEvent>(this);
    EventBus::subscribe<FrameDataCorrectionEvent>(this);
}

void MoveTimelineViewModel::on_event(const StatusEvent &event) {
    const std::scoped_lock lock(m_mutex);
    m_model.game_frame = event.game_frame;
}

void MoveTimelineViewModel::on_event(const FrameDataEvent &event) {
    const std::scoped_lock lock(m_mutex);
    update_from_frame_data(event.data);
}

void MoveTimelineViewModel::on_event(const FrameDataCorrectionEvent &event) {
    const std::scoped_lock lock(m_mutex);
    update_from_frame_data(event.data);
}

MoveTimelineModel MoveTimelineViewModel::snapshot() const {
    const std::scoped_lock lock(m_mutex);
    return m_model;
}

void MoveTimelineViewModel::update_from_frame_data(const FrameDataPoint &data) {
    const bool p1_attacker = data.p1.is_attacker;
    const PlayerFrameData &attacker = p1_attacker ? data.p1 : data.p2;
    const PlayerFrameData &defender = p1_attacker ? data.p2 : data.p1;

    const uint32_t startup = attacker.startup_frames > 0 ? static_cast<uint32_t>(attacker.startup_frames) : 0;
    const uint32_t start = data.game_frame >= startup ? data.game_frame - startup : 0;
    const uint32_t first_active = start + attacker.first_active_frame;
    const uint32_t last_active = std::max(start + attacker.last_active_frame, first_active);
    const uint32_t end = std::max(attacker.free_frame, last_active);

    const MoveTimeline attacker_timeline{
        .active = true,
        .start_frame = start,
        .first_active_frame = first_active,
        .last_active_frame = last_active,
        .end_frame = end,
    };

    MoveTimeline defender_timeline{};
    if (defender.free_frame != 0) {
        const uint32_t defender_start = data.game_frame;
        defender_timeline = {
            .active = true,
            .start_frame = defender_start,
            .first_active_frame = defender_start,
            .last_active_frame = defender_start,
            .end_frame = std::max(defender.free_frame, defender_start),
        };
    }

    if (p1_attacker) {
        m_model.p1 = attacker_timeline;
        if (defender_timeline.active) {
            m_model.p2 = defender_timeline;
        }
    } else {
        m_model.p2 = attacker_timeline;
        if (defender_timeline.active) {
            m_model.p1 = defender_timeline;
        }
    }
}
