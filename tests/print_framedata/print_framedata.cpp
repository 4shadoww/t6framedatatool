/*
  Copyright (C) 2025 Noa-Emil Nissinen

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

#include <iostream>

#include "game_state_reader.h"
#include "memory_reader_types.h"

// Just print the current frame data and exit

int main() {
    init_memory_reader();

    struct GameFrame state{};
    const int result = read_game_state(&state);
    if (result == READ_ERROR) {
        std::cout << "read failed" << std::endl;
        return -1;
    }

    std::cout << std::boolalpha;
    std::cout << "FRAME DATA" << std::endl;
    std::cout << "----------" << std::endl;

    // P1
    std::cout << "P1 last action: " << state.p1.frames_last_action << std::endl;
    std::cout << "P1 recovery frames: " << state.p1.recovery_frames << std::endl;
    std::cout << "P1 connection: " << (bool) state.p1.connection << std::endl;
    std::cout << "P1 intent: " << state.p1.intent << std::endl;
    std::cout << "P1 move: " << state.p1.move << std::endl;
    std::cout << "P1 state: " << state.p1.state << std::endl;
    std::cout << "P1 string type: " << state.p1.string_type << std::endl;
    std::cout << "P1 string state: " << state.p1.string_state << std::endl;
    std::cout << "P1 position: " << state.p1.position.x << ", " << state.p1.position.y << ", " << state.p1.position.z << std::endl;
    std::cout << "P1 seq: " << state.p1.attack_seq << std::endl;

    // P2
    std::cout << "P2 last action: " << state.p2.frames_last_action << std::endl;
    std::cout << "P2 recovery frames: " << state.p2.recovery_frames << std::endl;
    std::cout << "P2 connection: " << (bool) state.p2.connection << std::endl;
    std::cout << "P2 intent: " << state.p2.intent << std::endl;
    std::cout << "P2 move: " << state.p2.move << std::endl;
    std::cout << "P2 state: " << state.p2.state << std::endl;
    std::cout << "P2 string type: " << state.p2.string_type << std::endl;
    std::cout << "P2 string state: " << state.p2.string_state << std::endl;
    std::cout << "P2 position: " << state.p2.position.x << ", " << state.p2.position.y << ", " << state.p2.position.z << std::endl;
    std::cout << "P2 seq: " << state.p2.attack_seq << std::endl;

    // Game state
    std::cout << "current game frame: " << state.game_frame << std::endl;

    int side = 0;
    (void) player_side(&side);
    std::cout << "player side: " << side << std::endl;
    std::cout << "player side address: " << std::hex << player_side_address() << std::endl;

    return 0;
}
