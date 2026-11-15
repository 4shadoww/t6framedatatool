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

#include "replay_harness.hpp"

#include "event_bus.hpp"
#include "frame_dump_reader.hpp"


bool ReplayHarness::replay(const std::string &path) {
    FrameDumpReader reader;
    if (!reader.open(path)) {
        return false;
    }

    m_game_frame = 0;
    m_frame_count = 0;
    m_frame_data_events.clear();
    m_distance_events.clear();
    m_status_events.clear();

    EventBus::clear<FrameDataEvent>();
    EventBus::clear<DistanceEvent>();
    EventBus::clear<StatusEvent>();
    EventBus::subscribe<FrameDataEvent>(this);
    EventBus::subscribe<DistanceEvent>(this);
    EventBus::subscribe<StatusEvent>(this);
    FrameDataAnalyser::reset();

    FrameDumpRecord record{};
    while (reader.read(record)) {
        m_game_frame = record.frame.game_frame;

        if (!FrameDataAnalyser::process_frame(record.frame, record.player_side)) {
            return false;
        }

        m_frame_count++;
    }

    return true;
}

uint64_t ReplayHarness::frame_count() const {
    return m_frame_count;
}

const std::vector<RecordedFrameDataEvent> &ReplayHarness::frame_data_events() const {
    return m_frame_data_events;
}

const std::vector<RecordedDistanceEvent> &ReplayHarness::distance_events() const {
    return m_distance_events;
}

const std::vector<RecordedStatusEvent> &ReplayHarness::status_events() const {
    return m_status_events;
}

void ReplayHarness::on_event(const FrameDataEvent &event) {
    m_frame_data_events.push_back({.game_frame = m_game_frame, .data = event.data});
}

void ReplayHarness::on_event(const DistanceEvent &event) {
    m_distance_events.push_back({.game_frame = m_game_frame, .distance = event.distance});
}

void ReplayHarness::on_event(const StatusEvent &event) {
    m_status_events.push_back({.game_frame = m_game_frame, .p1 = event.p1, .p2 = event.p2});
}
