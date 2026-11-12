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

#include "viewmodel/frame_data_view_model.hpp"
#include "logging.h"

#define RESET_FRAME_DATA_MILLIS 2500

// Event receiver
void FrameDataViewModel::on_frame_data(const FrameDataPoint &frame_data) {
    m_model.last_update = std::chrono::steady_clock::now().time_since_epoch().count();
    m_model.frame_data = frame_data;

    const PlayerFrameData &local = frame_data.player_side == PlayerSide::LEFT ? frame_data.p1 : frame_data.p2;
    log_info("local player: P%d, startup frames: %d (%d~%d), frame advantage: %d, KD: %d",
             frame_data.player_side == PlayerSide::LEFT ? 1 : 2,
             local.startup_frames,
             local.first_active_frame,
             local.last_active_frame,
             local.frame_advantage,
             local.knock_down);
}
void FrameDataViewModel::on_distance(const float distance) {
    m_model.distance = distance;
}
void FrameDataViewModel::on_player_status(const PlayerState state) {
    m_model.status = state;
}
void FrameDataViewModel::on_game_hooked(const bool hooked) {
    m_model.game_hooked = hooked;
}

// Accessors for view
bool FrameDataViewModel::has_frame_data() const {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = now.time_since_epoch() - std::chrono::steady_clock::duration(m_model.last_update);
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    return millis <= RESET_FRAME_DATA_MILLIS;
}
