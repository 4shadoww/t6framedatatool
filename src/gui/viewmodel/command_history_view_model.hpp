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

#ifndef COMMAND_HISTORY_VIEW_MODEL_HPP
#define COMMAND_HISTORY_VIEW_MODEL_HPP

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>

#include "configuration.hpp"
#include "event_bus.hpp"
#include "events.hpp"
#include "frame_data_defs.hpp"
#include "ring_buffer.hpp"

constexpr std::size_t COMMAND_HISTORY_LIMIT = 25;
constexpr std::size_t COMMAND_HISTORY_PLAYER_COUNT = 2;
constexpr uint16_t COMMAND_HISTORY_MAX_FRAMES = 999;
constexpr auto COMMAND_HISTORY_MAX_AGE = std::chrono::seconds(10);

struct CommandHistoryEntry {
    InputState input{};
    uint16_t frames = 0;
    std::chrono::steady_clock::time_point timestamp;
};

struct CommandHistoryModel {
    RingBuffer<CommandHistoryEntry> entries{COMMAND_HISTORY_LIMIT};
};

struct CommandHistorySnapshot {
    std::array<CommandHistoryEntry, COMMAND_HISTORY_LIMIT> entries{};
    std::size_t count = 0;
};

class CommandHistoryViewModel {
public:
    explicit CommandHistoryViewModel(ConfigurationData *configuration) noexcept;

    void on_event(const InputEvent &event) {
        on_input(event.input, event.own_side);
    }
    void on_event(const GameHookedEvent & /*event*/) {
        on_game_hooked();
    }

    void on_input(InputState input, PlayerSide own_side);
    void on_game_hooked();

    [[nodiscard]] CommandHistorySnapshot snapshot(PlayerSide player) const;
    [[nodiscard]] PlayerSide panel_side(PlayerSide player) const;
    [[nodiscard]] bool show_command_history(PlayerSide player) const;

private:
    ConfigurationData *m_configuration;
    std::array<CommandHistoryModel, COMMAND_HISTORY_PLAYER_COUNT> m_models{};
    PlayerSide m_own_side = PlayerSide::LEFT;
    mutable std::mutex m_mutex;
};

#endif
