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

    FrameDataAnalyser::reset(this);

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

const std::vector<FrameDataEvent> &ReplayHarness::frame_data_events() const {
    return m_frame_data_events;
}

const std::vector<DistanceEvent> &ReplayHarness::distance_events() const {
    return m_distance_events;
}

const std::vector<StatusEvent> &ReplayHarness::status_events() const {
    return m_status_events;
}

void ReplayHarness::frame_data(const FrameDataPoint data) {
    m_frame_data_events.push_back({.game_frame = m_game_frame, .data = data});
}

void ReplayHarness::distance(const float distance) {
    m_distance_events.push_back({.game_frame = m_game_frame, .distance = distance});
}

void ReplayHarness::status(const PlayerState status) {
    m_status_events.push_back({.game_frame = m_game_frame, .status = status});
}

void ReplayHarness::input(const InputState /*input*/) {
    // NOP
}

void ReplayHarness::game_hooked() {
    // NOP
}
