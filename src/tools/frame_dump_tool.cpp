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

#include <cstring>
#include <iostream>
#include <string>

#include "frame_dump_reader.hpp"

namespace {

void print_usage(const char *const program) {
    std::cout << "usage: " << program << " [OPTIONS] <dump>\n"
              << "\nOptions:\n"
              << "  -h, --help\t\tprint help\n"
              << "  -f, --frames\t\tprint individual frames\n";
}

void print_metadata(const std::string &path, const FrameDumpReader &reader) {
    const FrameDumpHeader &header = reader.header();

    std::cout << "file: " << path << '\n';
    std::cout << "file size: " << reader.file_size() << " bytes\n";
    std::cout << "magic: ";
    std::cout.write(header.magic, sizeof(header.magic));
    std::cout << '\n';
    std::cout << "version: " << header.version << '\n';
    std::cout << "record size: " << header.record_size << '\n';
    std::cout << "frames: " << reader.frame_count() << '\n';
}

void print_player(const char *const name, const PlayerFrame &player) {
    std::cout << "  " << name << ":\n"
              << "    frames_last_action: " << player.frames_last_action << '\n'
              << "    recovery_frames: " << player.recovery_frames << '\n'
              << "    connection: " << static_cast<int>(player.connection) << '\n'
              << "    intent: " << player.intent << '\n'
              << "    move_id: " << player.move_id << '\n'
              << "    state: " << player.state << '\n'
              << "    position: " << player.position.x << ", " << player.position.y << ", " << player.position.z << '\n'
              << "    attack_seq: " << player.attack_seq << '\n'
              << "    move index: " << player.move.index << '\n'
              << "    move start: " << player.move.start_frame << '\n'
              << "    first active: " << player.move.first_active_frame << '\n'
              << "    last active: " << player.move.last_active_frame << '\n'
              << "    cancel startup: " << player.move.cancel_startup << '\n'
              << "    string next: " << player.move.string_next << '\n'
              << "    hitlevel: " << player.move.hitlevel << '\n'
              << "    anim_addr: " << player.move.anim_addr << '\n';
}

void print_frame(const uint64_t index, const FrameDumpRecord &record) {
    std::cout << "frame " << index << '\n'
              << "  player side: " << record.player_side << '\n'
              << "  game frame: " << record.frame.game_frame << '\n';

    print_player("p1", record.frame.p1);
    print_player("p2", record.frame.p2);
}

} // namespace

int main(const int argc, const char *const *const argv) {
    bool print_frames = false;
    const char *path = nullptr;

    for (int i = 1; i < argc; i++) {
        const char *const argument = argv[i];

        if (std::strcmp(argument, "-h") == 0 || std::strcmp(argument, "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }

        if (std::strcmp(argument, "-f") == 0 || std::strcmp(argument, "--frames") == 0) {
            print_frames = true;
            continue;
        }

        if (argument[0] == '-') {
            std::cerr << "unknown option: " << argument << '\n';
            print_usage(argv[0]);
            return 1;
        }

        if (path != nullptr) {
            std::cerr << "too many files given" << '\n';
            print_usage(argv[0]);
            return 1;
        }

        path = argument;
    }

    if (path == nullptr) {
        print_usage(argv[0]);
        return 1;
    }

    FrameDumpReader reader;
    if (!reader.open(path)) {
        std::cerr << "failed to open or parse frame dump: " << path << '\n';
        return 1;
    }

    print_metadata(path, reader);

    if (print_frames) {
        FrameDumpRecord record{};
        uint64_t index = 0;

        while (reader.read(record)) {
            print_frame(index, record);
            index++;
        }
    }

    return 0;
}
