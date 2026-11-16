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

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "input_decoding.hpp"
#include "input_dump_reader.hpp"

namespace {

const char *direction_name(const InputDirection direction) {
    switch (direction) {
    case InputDirection::NEUTRAL:
        return "neutral";
    case InputDirection::UP:
        return "up";
    case InputDirection::DOWN:
        return "down";
    case InputDirection::BACK:
        return "back";
    case InputDirection::UP_BACK:
        return "up-back";
    case InputDirection::DOWN_BACK:
        return "down-back";
    case InputDirection::FORWARD:
        return "forward";
    case InputDirection::UP_FORWARD:
        return "up-forward";
    case InputDirection::DOWN_FORWARD:
        return "down-forward";
    default:
        return "unknown";
    }
}

void print_usage(const char *const program) {
    std::cout << "usage: " << program << " [OPTIONS] <dump>\n"
              << "\nOptions:\n"
              << "  -h, --help\t\tprint help\n"
              << "  -f, --frames\t\tprint individual frames\n";
}

void print_metadata(const std::string &path, const InputDumpReader &reader) {
    const InputDumpHeader &header = reader.header();

    std::cout << "file: " << path << '\n';
    std::cout << "file size: " << reader.file_size() << " bytes\n";
    std::cout << "magic: ";
    std::cout.write(header.magic, sizeof(header.magic));
    std::cout << '\n';
    std::cout << "version: " << header.version << '\n';
    std::cout << "record size: " << header.record_size << '\n';
    std::cout << "frames: " << reader.frame_count() << '\n';
}

void print_frame(const uint64_t index, const InputDumpRecord &record) {
    const InputDirection direction = input_decoding::direction(record.raw);
    const uint8_t buttons = input_decoding::buttons(record.raw);

    std::cout << "frame " << index << '\n'
              << "  game frame: " << record.game_frame << '\n'
              << "  raw: 0x" << std::hex << record.raw << std::dec << '\n'
              << "  direction: " << static_cast<uint32_t>(direction) << " (" << direction_name(direction) << ")\n"
              << "  buttons: 0x" << std::hex << static_cast<uint32_t>(buttons) << std::dec << '\n';
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

    InputDumpReader reader;
    if (!reader.open(path)) {
        std::cerr << "failed to open or parse input dump: " << path << '\n';
        return 1;
    }

    print_metadata(path, reader);

    if (print_frames) {
        InputDumpRecord record{};
        uint64_t index = 0;

        while (reader.read(record)) {
            print_frame(index, record);
            index++;
        }
    }

    return 0;
}
