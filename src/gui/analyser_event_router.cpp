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

#include "analyser_event_router.hpp"
#include "platform_gui.hpp"

void AnalyserEventRouter::frame_data(const FrameDataPoint frame_data) {
    m_frame_data->on_frame_data(frame_data);
}

void AnalyserEventRouter::distance(const float distance) {
    m_frame_data->on_distance(distance);
}

void AnalyserEventRouter::status(const PlayerState state) {
    m_frame_data->on_player_status(state);
}

void AnalyserEventRouter::input(const InputState /*input*/) {
    // NOP
}

void AnalyserEventRouter::game_hooked() {
    m_frame_data->on_game_hooked(true);
    platform_find_game_window();
}
