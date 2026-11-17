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

#ifndef FRAME_DATA_ASSERTIONS_HPP
#define FRAME_DATA_ASSERTIONS_HPP

#include <type_traits>
#include <vector>

#include "replay_harness.hpp"

struct ExpectedFrameData {
    PlayerFrameData p1;
    PlayerFrameData p2;

    ExpectedFrameData(PlayerFrameData p1_data, PlayerFrameData p2_data);
};

ExpectedFrameData event(int p1_startup,
                        uint32_t p1_first_active,
                        uint32_t p1_last_active,
                        int p1_advantage,
                        bool p1_knock_down,
                        int p2_startup,
                        uint32_t p2_first_active,
                        uint32_t p2_last_active,
                        int p2_advantage,
                        bool p2_knock_down);

struct ExpectedCorrection {
    uint32_t game_frame;
    uint32_t corrected_game_frame;
    ExpectedFrameData data;
};

ExpectedCorrection correction(uint32_t game_frame, uint32_t corrected_game_frame, ExpectedFrameData data);

void expect_frame_data_sequence(const std::vector<RecordedFrameDataEvent> &actual,
                                const std::vector<ExpectedFrameData> &expected);

void expect_frame_data_correction_sequence(const std::vector<RecordedFrameDataEvent> &actual,
                                           const std::vector<ExpectedCorrection> &expected);

template<typename... T>
void expect_frame_data_events(const std::vector<RecordedFrameDataEvent> &actual, const T &...events) {
    static_assert((std::is_same_v<std::decay_t<T>, ExpectedFrameData> && ...),
                  "expected events must be ExpectedFrameData");
    expect_frame_data_sequence(actual, {events...});
}

template<typename... T>
void expect_frame_data_correction_events(const std::vector<RecordedFrameDataEvent> &actual, const T &...corrections) {
    static_assert((std::is_same_v<std::decay_t<T>, ExpectedCorrection> && ...),
                  "expected corrections must be ExpectedCorrection");
    expect_frame_data_correction_sequence(actual, {corrections...});
}

#endif
