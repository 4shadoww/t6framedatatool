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

#include "viewmodel/command_history_view_model.hpp"

#include <algorithm>

namespace {

constexpr PlayerSide opposite(const PlayerSide side) {
    return side == PlayerSide::LEFT ? PlayerSide::RIGHT : PlayerSide::LEFT;
}

} // namespace

CommandHistoryViewModel::CommandHistoryViewModel(ConfigurationData *configuration) noexcept
    : m_configuration(configuration) {
    EventBus::subscribe<InputEvent>(this);
    EventBus::subscribe<GameHookedEvent>(this);
}

bool CommandHistoryViewModel::show_command_history(const PlayerSide player) const {
    if (player == PlayerSide::RIGHT) {
        return m_configuration->ui.show_player2_command_history;
    }

    return m_configuration->ui.show_command_history;
}

PlayerSide CommandHistoryViewModel::panel_side(const PlayerSide player) const {
    const std::scoped_lock lock(m_mutex);

    if (player == PlayerSide::LEFT) {
        return m_own_side;
    }

    return opposite(m_own_side);
}

void CommandHistoryViewModel::on_input(const InputState input, const PlayerSide own_side) {
    const std::scoped_lock lock(m_mutex);

    m_own_side = own_side;
    CommandHistoryModel &model = m_models.at(static_cast<std::size_t>(input.side));
    const auto now = std::chrono::steady_clock::now();

    while (model.entries.item_count() > 1 && now - model.entries.tail()->timestamp > COMMAND_HISTORY_MAX_AGE) {
        model.entries.pop();
    }

    if (model.entries.item_count() == 0) {
        model.entries.push({.input = input, .frames = 1, .timestamp = now});
        return;
    }

    CommandHistoryEntry *const last = model.entries.head();
    if (last->input.direction == input.direction && last->input.buttons == input.buttons) {
        const uint32_t frames = input.game_frame - last->input.game_frame + 1;
        last->frames = static_cast<uint16_t>(std::min(frames, static_cast<uint32_t>(COMMAND_HISTORY_MAX_FRAMES)));
        return;
    }

    model.entries.push({.input = input, .frames = 1, .timestamp = now});
}

void CommandHistoryViewModel::on_game_hooked() {
    const std::scoped_lock lock(m_mutex);
    for (CommandHistoryModel &model : m_models) {
        model.entries.clear();
    }
}

CommandHistorySnapshot CommandHistoryViewModel::snapshot(const PlayerSide player) const {
    const std::scoped_lock lock(m_mutex);

    const CommandHistoryModel &model = m_models.at(static_cast<std::size_t>(player));
    CommandHistorySnapshot result;
    result.count = model.entries.item_count();

    for (std::size_t i = 0; i < result.count; i++) {
        result.entries.at(i) = *model.entries.get(i);
    }

    return result;
}
