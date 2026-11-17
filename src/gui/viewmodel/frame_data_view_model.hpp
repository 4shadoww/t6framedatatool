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

#include "event_bus.hpp"
#include "events.hpp"
#include "frame_data_defs.hpp"

struct FrameDataModel {
    FrameDataPoint frame_data;
    float distance;
    PlayerState p1_status;
    PlayerState p2_status;
    bool game_hooked;
    bool corrected;
    std::chrono::steady_clock::rep last_update;
};

class FrameDataViewModel {
public:
    FrameDataViewModel() noexcept {
        EventBus::subscribe<FrameDataEvent>(this);
        EventBus::subscribe<FrameDataCorrectionEvent>(this);
        EventBus::subscribe<DamageDealtEvent>(this);
        EventBus::subscribe<DistanceEvent>(this);
        EventBus::subscribe<StatusEvent>(this);
        EventBus::subscribe<GameHookedEvent>(this);
    }

    // Event receiver
    void on_event(const FrameDataEvent &event) {
        on_frame_data(event.data);
    }
    void on_event(const FrameDataCorrectionEvent &event) {
        on_frame_data_correction(event.data);
    }
    void on_event(const DamageDealtEvent &event) {
        on_damage_dealt(event.damage, event.side);
    }
    void on_event(const DistanceEvent &event) {
        on_distance(event.distance);
    }
    void on_event(const StatusEvent &event) {
        on_player_status(event);
    }
    void on_event(const GameHookedEvent & /*event*/) {
        on_game_hooked(true);
    }

    void on_frame_data(const FrameDataPoint &frame_data);
    void on_frame_data_correction(const FrameDataPoint &frame_data);
    void on_damage_dealt(uint32_t damage, PlayerSide side);
    void on_distance(const float distance);
    void on_player_status(const StatusEvent &status);
    void on_game_hooked(const bool hooked);

    // Accessors for view
    [[nodiscard]] bool has_frame_data() const;
    [[nodiscard]] const FrameDataPoint &get_frame_data() const {
        return m_model.frame_data;
    }
    [[nodiscard]] float get_distance() const {
        return m_model.distance;
    }
    [[nodiscard]] PlayerState get_player_state(const PlayerSide side) const {
        return side == PlayerSide::LEFT ? m_model.p1_status : m_model.p2_status;
    }
    [[nodiscard]] bool is_game_hooked() const {
        return m_model.game_hooked;
    }
    [[nodiscard]] bool is_corrected() const {
        return m_model.corrected;
    }

private:
    FrameDataModel m_model{};
};

#endif
