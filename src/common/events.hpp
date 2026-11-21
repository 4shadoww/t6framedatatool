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

#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <cstdint>

#include "frame_data_defs.hpp"

struct FrameDataEvent {
    FrameDataPoint data;
};

struct FrameDataCorrectionEvent {
    FrameDataPoint data;
};

struct DamageDealtEvent {
    uint32_t damage;
    PlayerSide side;
};

struct DistanceEvent {
    float distance;
};

struct StatusEvent {
    PlayerState p1;
    PlayerState p2;
    bool in_practice;
};

struct InputEvent {
    InputState input;
    // Side the local player is on
    PlayerSide own_side;
};

struct GameHookedEvent {};

#endif
