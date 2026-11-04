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

#include "frame_dump_reader.hpp"

#include <array>
#include <cstring>

namespace {

template<typename T>
bool read_value(std::ifstream &file, T &value) {
    std::array<char, sizeof(T)> bytes{};
    file.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));

    if (file.gcount() != static_cast<std::streamsize>(bytes.size())) {
        return false;
    }

    std::memcpy(&value, bytes.data(), sizeof(T));
    return true;
}

} // namespace

bool FrameDumpReader::open(const std::string &path) {
    close();

    m_file.open(path, std::ios::binary);
    if (!m_file.is_open()) {
        return false;
    }

    m_file.seekg(0, std::ios::end);
    const std::streamoff file_size = m_file.tellg();
    if (file_size < 0 || static_cast<uint64_t>(file_size) < sizeof(FrameDumpHeader)) {
        close();
        return false;
    }

    m_file.seekg(0, std::ios::beg);
    if (!read_value(m_file, m_header)) {
        close();
        return false;
    }

    if (std::memcmp(m_header.magic, FRAME_DUMP_MAGIC, sizeof(m_header.magic)) != 0 ||
        m_header.version != FRAME_DUMP_VERSION || m_header.record_size != sizeof(FrameDumpRecord)) {
        close();
        return false;
    }

    m_file_size = static_cast<uint64_t>(file_size);

    const uint64_t payload_size = m_file_size - sizeof(FrameDumpHeader);
    if (payload_size % m_header.record_size != 0) {
        close();
        return false;
    }

    m_frame_count = payload_size / m_header.record_size;

    return true;
}

void FrameDumpReader::close() {
    if (m_file.is_open()) {
        m_file.close();
    }
    m_file.clear();
    m_header = {};
    m_frame_count = 0;
    m_file_size = 0;
}

bool FrameDumpReader::is_open() const {
    return m_file.is_open();
}

const FrameDumpHeader &FrameDumpReader::header() const {
    return m_header;
}

uint64_t FrameDumpReader::frame_count() const {
    return m_frame_count;
}

uint64_t FrameDumpReader::file_size() const {
    return m_file_size;
}

bool FrameDumpReader::read(FrameDumpRecord &record) {
    if (!m_file.is_open()) {
        return false;
    }

    return read_value(m_file, record);
}

bool FrameDumpReader::rewind() {
    if (!m_file.is_open()) {
        return false;
    }

    m_file.clear();
    m_file.seekg(static_cast<std::streamoff>(sizeof(FrameDumpHeader)), std::ios::beg);

    return m_file.good();
}
