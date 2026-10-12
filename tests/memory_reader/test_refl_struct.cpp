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

#include <gtest/gtest.h>

#include "moveset_structs.h"
#include "refl_struct.h"

namespace {

void store_be16(unsigned char *dst, uint16_t value) {
    const unsigned bits = value;
    dst[0] = static_cast<unsigned char>(bits >> 8U);
    dst[1] = static_cast<unsigned char>(bits);
}

void store_be32(unsigned char *dst, uint32_t value) {
    dst[0] = static_cast<unsigned char>(value >> 24U);
    dst[1] = static_cast<unsigned char>(value >> 16U);
    dst[2] = static_cast<unsigned char>(value >> 8U);
    dst[3] = static_cast<unsigned char>(value);
}

} // namespace

TEST(refl_struct, swaps_move_fields_and_leaves_marked_fields) {
    unsigned char raw[sizeof(Move)] = {};
    Move move{};

    store_be32(raw + offsetof(Move, o_0x0_int), 0x008420B0);
    store_be32(raw + offsetof(Move, cancel_addr), 0x362B25C0);
    store_be16(raw + offsetof(Move, transition), 0x8001);
    store_be16(raw + offsetof(Move, move_id_val1), 9);
    store_be32(raw + offsetof(Move, anim_len), 30);
    store_be32(raw + offsetof(Move, hitbox_location), 0x0D000000);
    store_be32(raw + offsetof(Move, first_active_frame), 7);
    store_be32(raw + offsetof(Move, last_active_frame), 10);
    raw[0x1E] = 0xAB;
    raw[0x1F] = 0xCD;

    unsigned char saved[sizeof(raw)];
    std::memcpy(saved, raw, sizeof(raw));
    ASSERT_EQ(load_refl_struct(&move, raw, sizeof(raw), move_fields, move_field_count), REFL_OK);
    EXPECT_EQ(std::memcmp(raw, saved, sizeof(raw)), 0);

    EXPECT_EQ(std::memcmp(&move.o_0x0_int, raw + offsetof(Move, o_0x0_int), 4), 0);
    EXPECT_EQ(move.cancel_addr, 0x362B25C0U);
    EXPECT_EQ(move.transition, 0x8001);
    EXPECT_EQ(move.move_id_val1, 9);
    EXPECT_EQ(move.anim_len, 30);
    EXPECT_EQ(std::memcmp(&move.hitbox_location, raw + offsetof(Move, hitbox_location), 4), 0);
    EXPECT_EQ(move.first_active_frame, 7U);
    EXPECT_EQ(move.last_active_frame, 10U);
    unsigned char pad[2] = {};
    const void *move_bytes = &move;
    std::memcpy(pad, static_cast<const char *>(move_bytes) + 0x1E, sizeof(pad));
    EXPECT_EQ(pad[0], 0xAB);
    EXPECT_EQ(pad[1], 0xCD);
}

TEST(refl_struct, swaps_cancel_halves_separately) {
    unsigned char raw[sizeof(Cancel)] = {};
    Cancel cancel{};

    store_be32(raw + offsetof(Cancel, input.direction), 0x00008000);
    store_be32(raw + offsetof(Cancel, detection_end), 32767);
    store_be32(raw + offsetof(Cancel, starting_frame), 27);
    store_be16(raw + offsetof(Cancel, move_id), 0x8001);
    store_be16(raw + offsetof(Cancel, cancel_option), 0x0150);

    ASSERT_EQ(load_refl_struct(&cancel, raw, sizeof(raw), cancel_fields, cancel_field_count), REFL_OK);

    EXPECT_EQ(cancel.input.direction, 0x00008000U);
    EXPECT_EQ(cancel.input.button, 0U);
    EXPECT_EQ(cancel.detection_end, 32767U);
    EXPECT_EQ(cancel.starting_frame, 27U);
    EXPECT_EQ(cancel.move_id, 0x8001);
    EXPECT_EQ(cancel.cancel_option, 0x0150);
}

TEST(refl_struct, swaps_uint64_field) {
    struct Value64 {
        uint64_t value;
    };
    const Field fields[] = {F64(Value64, value, 1)};
    unsigned char raw[sizeof(Value64)] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Value64 value{};

    ASSERT_EQ(load_refl_struct(&value, raw, sizeof(raw), fields, 1), REFL_OK);
    EXPECT_EQ(value.value, 0x0102030405060708ULL);
}

TEST(refl_struct, rejects_a_field_past_the_end) {
    uint32_t value = 0;
    unsigned char raw[sizeof(value)] = {};
    const Field fields[] = {{.name = "value", .offset = 2, .width = 4, .swap = 1}};

    EXPECT_EQ(load_refl_struct(&value, raw, sizeof(raw), fields, 1), REFL_ERROR);
}

TEST(refl_struct, formats_fields_into_a_string) {
    struct Value {
        uint32_t amount;
    };
    const Field fields[] = {F32(Value, amount, 1)};
    Value value{};
    char text[128] = {};

    value.amount = 7;
    const int length = format_refl_struct(text, sizeof(text), "Value", &value, fields, 1);

    ASSERT_GT(length, 0);
    EXPECT_LT(length, static_cast<int>(sizeof(text)));
    EXPECT_NE(std::strstr(text, "Value\n"), nullptr);
    EXPECT_NE(std::strstr(text, "amount = 7"), nullptr);
}
