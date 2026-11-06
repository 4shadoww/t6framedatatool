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

#include "frame_data_assertions.hpp"

#include <cstddef>

#include <gtest/gtest.h>

namespace {

PlayerFrameData make_player_frame_data(const int startup,
                                       const uint32_t first_active,
                                       const uint32_t last_active,
                                       const int advantage,
                                       const bool knock_down) {
    PlayerFrameData data{};
    data.startup_frames = startup;
    data.first_active_frame = first_active;
    data.last_active_frame = last_active;
    data.frame_advantage = advantage;
    data.knock_down = knock_down;
    return data;
}

void expect_player_frame_data(const PlayerFrameData &actual, const PlayerFrameData &expected) {
    EXPECT_EQ(actual.startup_frames, expected.startup_frames);
    EXPECT_EQ(actual.first_active_frame, expected.first_active_frame);
    EXPECT_EQ(actual.last_active_frame, expected.last_active_frame);
    EXPECT_EQ(actual.frame_advantage, expected.frame_advantage);
    EXPECT_EQ(actual.knock_down, expected.knock_down);
}

} // namespace

ExpectedFrameData::ExpectedFrameData(const PlayerFrameData p1_data, const PlayerFrameData p2_data)
    : p1(p1_data)
    , p2(p2_data) {}

ExpectedFrameData event(const int p1_startup,
                        const uint32_t p1_first_active,
                        const uint32_t p1_last_active,
                        const int p1_advantage,
                        const bool p1_knock_down,
                        const int p2_startup,
                        const uint32_t p2_first_active,
                        const uint32_t p2_last_active,
                        const int p2_advantage,
                        const bool p2_knock_down) {
    return {make_player_frame_data(p1_startup, p1_first_active, p1_last_active, p1_advantage, p1_knock_down),
            make_player_frame_data(p2_startup, p2_first_active, p2_last_active, p2_advantage, p2_knock_down)};
}

void expect_frame_data_sequence(const std::vector<FrameDataEvent> &actual,
                                const std::vector<ExpectedFrameData> &expected) {
    ASSERT_EQ(actual.size(), expected.size());

    for (std::size_t i = 0; i < expected.size(); i++) {
        SCOPED_TRACE(testing::Message() << "event " << i);
        expect_player_frame_data(actual.at(i).data.p1, expected.at(i).p1);
        expect_player_frame_data(actual.at(i).data.p2, expected.at(i).p2);
    }
}
