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
#include "test_paths.hpp"

TEST(frame_data_analyser, king_3_4_1_reports_knockdown) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_king_34_3_kd.t6dump")));

    // FIXME: advantage does not match CSV 3+4,1 block -5.
    expect_frame_data_events(harness.frame_data_events(), event(19, 19U, 20U, -31, false, 0, 0U, 0U, 31, true));
}

TEST(frame_data_analyser, alisa_u4_reports_knockdown) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_alisa_u4_kd.t6dump")));

    // FIXME: advantage does not match CSV u+4 block -15~-14.
    expect_frame_data_events(harness.frame_data_events(), event(15, 15U, 16U, 14, false, 0, 0U, 0U, -14, true));
}

TEST(frame_data_analyser, alisa_des_1_reports_knockdown) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_alisa_des_1_kd.t6dump")));

    // FIXME: advantage does not match CSV DES.1 block -9.
    expect_frame_data_events(harness.frame_data_events(),
                             event(15, 14U, 40U, 19, false, 0, 0U, 0U, -19, false),
                             event(15, 14U, 40U, 23, false, 0, 0U, 0U, -23, true));
}

TEST(frame_data_analyser, alisa_des_1_block) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_alisa_des_1_block.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(15, 14U, 40U, -9, false, 0, 0U, 0U, 9, false));
}

TEST(frame_data_analyser, jin_1_hit) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_jin_1_hit.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(10, 10U, 10U, 8, false, 0, 0U, 0U, -8, false));
}

TEST(frame_data_analyser, jin_1_block) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_jin_1_block.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(10, 10U, 10U, 1, false, 0, 0U, 0U, -1, false));
}

TEST(frame_data_analyser, jin_hellsweep_kd) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_jin_f_n_d_df-4_kd.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(20, 20U, 21U, -29, false, 0, 0U, 0U, 29, true));
}

TEST(frame_data_analyser, jin_hellsweep_block) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_jin_f_n_d_df-4_block.t6dump")));

    expect_frame_data_events(harness.frame_data_events(), event(20, 20U, 21U, -31, false, 0, 0U, 0U, 31, false));
}

TEST(frame_data_analyser, king_d_3_4_hit) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_king_d-3-4_hit.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(16, 16U, 16U, -13, false, 0, 0U, 0U, 13, false));
}

TEST(frame_data_analyser, king_d_3_4_4_hit) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_king_d-3-4_4_hit.t6dump")));
    expect_frame_data_events(harness.frame_data_events(),
                             event(16, 16U, 16U, -13, false, 0, 0U, 0U, 13, false),
                             event(16, 16U, 16U, 12, false, 0, 0U, 0U, -12, false));
}

TEST(frame_data_analyser, yoshimitsu_db_2_block) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_yoshimitsu_db-2_block.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(15, 15U, 15U, -5, false, 0, 0U, 0U, 5, false));
}

TEST(frame_data_analyser, yoshimitsu_dgf_2_hit) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_yoshimitsu_dgf-2_hit.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(24, 24U, 28U, -31, false, 0, 0U, 0U, 31, true));
}

TEST(frame_data_analyser, alisa_ub_1_2_hit) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_alisa_ub_1-2_hit.t6dump")));
    expect_frame_data_events(harness.frame_data_events(),
                             event(20, 20U, 21U, -33, false, 0, 0U, 0U, 33, true),
                             event(20, 17U, 22U, -39, false, 0, 0U, 0U, 39, true));
}

TEST(frame_data_analyser, alisa_ub_1_2_block) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_alisa_ub_1-2_block.t6dump")));
    expect_frame_data_events(harness.frame_data_events(),
                             event(20, 20U, 21U, -8, false, 0, 0U, 0U, 8, false),
                             event(17, 17U, 22U, -1, false, 0, 0U, 0U, 1, false));
}

TEST(frame_data_analyser, alisa_des_f_f_1_2_hit) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_alisa_des_f_f_1-2_hit.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(12, 12U, 30U, -55, false, 0, 0U, 0U, 55, true));
}

TEST(frame_data_analyser, alisa_des_f_f_1_2_block) {
    ReplayHarness harness;

    ASSERT_TRUE(harness.replay(test_data("t6_alisa_des_f_f_1-2_block.t6dump")));
    expect_frame_data_events(harness.frame_data_events(), event(12, 12U, 30U, -5, false, 0, 0U, 0U, 5, false));
}
