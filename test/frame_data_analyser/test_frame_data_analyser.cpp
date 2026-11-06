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

#include "frame_data_assertions.hpp"
#include "replay_harness.hpp"

TEST(frame_data_analyser, king_3_4_1_reports_knockdown) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(TEST_DATA_DIR "/t6_king_34_3_hit.t6dump"));

    // FIXME: only startup and knockdown match CSV 3+4,1; active window and advantage come from replay.
    expect_frame_data_events(harness.frame_data_events(), event(19, 19U, 20U, -31, false, 0, 0U, 0U, 31, true));
}
