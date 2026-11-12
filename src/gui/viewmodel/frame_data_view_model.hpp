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

#ifndef FRAME_DATA_VIEWMODEL_HPP
#define FRAME_DATA_VIEWMODEL_HPP

#include <chrono>

#include "frame_data_defs.hpp"

struct FrameDataModel {
    FrameDataPoint frame_data;
    float distance;
    PlayerState status;
    bool game_hooked;
    std::chrono::steady_clock::rep last_update;
};

class FrameDataViewModel {
public:
    // Event receiver
    void on_frame_data(const FrameDataPoint &frame_data);
    void on_distance(const float distance);
    void on_player_status(const PlayerState state);
    void on_game_hooked(const bool hooked);

    // Accessors for view
    [[nodiscard]] bool has_frame_data() const;
    [[nodiscard]] const FrameDataPoint &get_frame_data() const {
        return m_model.frame_data;
    }
    [[nodiscard]] float get_distance() const {
        return m_model.distance;
    }
    [[nodiscard]] PlayerState get_player_state() const {
        return m_model.status;
    }
    [[nodiscard]] bool is_game_hooked() const {
        return m_model.game_hooked;
    }

private:
    FrameDataModel m_model;
};

#endif
