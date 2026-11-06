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

#ifndef REPLAY_HARNESS_HPP
#define REPLAY_HARNESS_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "frame_data_analyser.hpp"

struct FrameDataEvent {
    uint32_t game_frame;
    FrameDataPoint data;
};

struct DistanceEvent {
    uint32_t game_frame;
    float distance;
};

struct StatusEvent {
    uint32_t game_frame;
    PlayerState status;
};

class ReplayHarness : public EventListener {
public:
    bool replay(const std::string &path);

    [[nodiscard]] uint64_t frame_count() const;
    [[nodiscard]] const std::vector<FrameDataEvent> &frame_data_events() const;
    [[nodiscard]] const std::vector<DistanceEvent> &distance_events() const;
    [[nodiscard]] const std::vector<StatusEvent> &status_events() const;

    void frame_data(FrameDataPoint data) override;
    void distance(float distance) override;
    void status(PlayerState status) override;
    void game_hooked() override;

private:
    uint32_t m_game_frame = 0;
    uint64_t m_frame_count = 0;
    std::vector<FrameDataEvent> m_frame_data_events;
    std::vector<DistanceEvent> m_distance_events;
    std::vector<StatusEvent> m_status_events;
};

#endif
