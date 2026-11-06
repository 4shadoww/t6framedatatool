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

#include <gtest/gtest.h>

#include "frame_data_analyser.hpp"
#include "replay_harness.hpp"
#include "test_paths.hpp"

TEST(frame_data_analyser, replay_metadata) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_king_34_3_kd.t6dump")));
    EXPECT_EQ(harness.frame_count(), 169U);
    ASSERT_EQ(harness.distance_events().size(), 169U);
    ASSERT_EQ(harness.status_events().size(), 169U);
    ASSERT_EQ(harness.frame_data_events().size(), 1U);

    const FrameDataEvent &event = harness.frame_data_events().front();
    EXPECT_EQ(event.game_frame, 272929U);
    EXPECT_EQ(event.data.player_side, PlayerSide::LEFT);
    EXPECT_NEAR(harness.distance_events().front().distance, 1.84F, 0.01F);
    EXPECT_EQ(harness.status_events().front().status, PlayerState::STANDING);
}
