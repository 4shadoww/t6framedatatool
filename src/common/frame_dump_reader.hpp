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

#ifndef FRAME_DUMP_READER_HPP
#define FRAME_DUMP_READER_HPP

#include <cstdint>
#include <fstream>
#include <string>

#include "frame_dumper.h"

class FrameDumpReader {
public:
    bool open(const std::string &path);
    void close();
    [[nodiscard]] bool is_open() const;
    [[nodiscard]] const FrameDumpHeader &header() const;
    [[nodiscard]] uint64_t frame_count() const;
    [[nodiscard]] uint64_t file_size() const;
    bool read(FrameDumpRecord &record);
    bool rewind();

private:
    std::ifstream m_file;
    FrameDumpHeader m_header{};
    uint64_t m_frame_count = 0;
    uint64_t m_file_size = 0;
};

#endif
