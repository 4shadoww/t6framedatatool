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

#include <gtest/gtest.h>
#include <memory>

#include "ring_buffer.hpp"

#define BUFFER_SIZE 5

namespace {

class TestRingBuffer : public testing::Test {
protected:
    std::unique_ptr<RingBuffer<int>> m_ring_buffer;

    void SetUp() override {
        m_ring_buffer = std::make_unique<RingBuffer<int>>(BUFFER_SIZE);
    }

    void populate();
    void pop_all();
};

void TestRingBuffer::populate() { // NOLINT(readability-function-cognitive-complexity)
    for (int i = 1; i < 6; i++) {
        m_ring_buffer->push(i);
        ASSERT_EQ(i, m_ring_buffer->item_count());
        ASSERT_EQ(i, *m_ring_buffer->get(i - 1));
        ASSERT_EQ(i, *m_ring_buffer->get_abs(i - 1));
        ASSERT_EQ(i, *m_ring_buffer->get_from_head(0));
        if (i != 1) {
            ASSERT_EQ(i - 1, *m_ring_buffer->get_from_head(1));
        }
    }
}

void TestRingBuffer::pop_all() { // NOLINT(readability-function-cognitive-complexity)
    for (int i = 1; i < 6; i++) {
        ASSERT_EQ(i, m_ring_buffer->pop());
        ASSERT_EQ(4, m_ring_buffer->head_index());
        if (i != 5) {
            ASSERT_EQ(i, m_ring_buffer->tail_index());
        } else {
            ASSERT_EQ(4, m_ring_buffer->tail_index());
        }
        ASSERT_EQ(5 - i, m_ring_buffer->item_count());
    }

    // When empty returns default constructor
    ASSERT_EQ(0, m_ring_buffer->pop());
    ASSERT_EQ(4, m_ring_buffer->head_index());
    ASSERT_EQ(4, m_ring_buffer->tail_index());
    ASSERT_EQ(0, m_ring_buffer->item_count());
}

TEST_F(TestRingBuffer, initial_state) {
    ASSERT_EQ(0, m_ring_buffer->head_index());
    ASSERT_EQ(0, m_ring_buffer->tail_index());
    ASSERT_EQ(0, m_ring_buffer->item_count());
    EXPECT_EQ(BUFFER_SIZE, m_ring_buffer->capacity());
}

TEST_F(TestRingBuffer, push_full) {
    populate();
    ASSERT_EQ(5, m_ring_buffer->item_count());
    ASSERT_EQ(4, m_ring_buffer->head_index());
    ASSERT_EQ(0, m_ring_buffer->tail_index());
}

TEST_F(TestRingBuffer, push_overfill) { // NOLINT(readability-function-cognitive-complexity)
    populate();

    for (int i = 6; i < 11; i++) {
        m_ring_buffer->push(i);
        ASSERT_EQ(5, m_ring_buffer->item_count());
        ASSERT_EQ(i - 6, m_ring_buffer->head_index());
        ASSERT_EQ((i - 5) % 5, m_ring_buffer->tail_index());

        ASSERT_EQ(i, *m_ring_buffer->get(4));
        ASSERT_EQ(i, *m_ring_buffer->get_abs(i - 6));
        ASSERT_EQ(i, *m_ring_buffer->get_from_head(0));
        ASSERT_EQ(i - 4, *m_ring_buffer->get_from_head(4));
    }
}

TEST_F(TestRingBuffer, get_tail_head) {
    populate();
    ASSERT_EQ(1, *m_ring_buffer->tail());
    ASSERT_EQ(5, *m_ring_buffer->head());

    m_ring_buffer->push(6);
    ASSERT_EQ(2, *m_ring_buffer->tail());
    ASSERT_EQ(6, *m_ring_buffer->head());

    m_ring_buffer->push(7);
    ASSERT_EQ(3, *m_ring_buffer->tail());
    ASSERT_EQ(7, *m_ring_buffer->head());
}

TEST_F(TestRingBuffer, empty_get_bounds) {
    ASSERT_EQ(nullptr, m_ring_buffer->get(0));
    ASSERT_EQ(nullptr, m_ring_buffer->get(4));
    ASSERT_EQ(nullptr, m_ring_buffer->get_abs(0));
    ASSERT_EQ(nullptr, m_ring_buffer->get_abs(4));
    ASSERT_EQ(nullptr, m_ring_buffer->get_from_head(0));
    ASSERT_EQ(nullptr, m_ring_buffer->get_from_head(4));
    ASSERT_EQ(nullptr, m_ring_buffer->tail());
    ASSERT_EQ(nullptr, m_ring_buffer->head());
}

TEST_F(TestRingBuffer, get_bounds) {
    populate();
    // When has data, cannot go out-of-bounds
    ASSERT_NE(nullptr, m_ring_buffer->get(0));
    ASSERT_NE(nullptr, m_ring_buffer->get(BUFFER_SIZE));
    ASSERT_NE(nullptr, m_ring_buffer->get_from_head(0));
    ASSERT_NE(nullptr, m_ring_buffer->get_from_head(BUFFER_SIZE));

    ASSERT_NE(nullptr, m_ring_buffer->get_abs(0));
    ASSERT_EQ(nullptr, m_ring_buffer->get_abs(BUFFER_SIZE));
}

TEST_F(TestRingBuffer, clear) {
    populate();
    m_ring_buffer->clear();

    ASSERT_EQ(0, m_ring_buffer->head_index());
    ASSERT_EQ(0, m_ring_buffer->tail_index());
    ASSERT_EQ(0, m_ring_buffer->item_count());
}

TEST_F(TestRingBuffer, pop) {
    populate();
    pop_all();
}

TEST_F(TestRingBuffer, repopulate) {
    populate();
    pop_all();

    for (int i = 1; i < 6; i++) {
        m_ring_buffer->push(i);
        ASSERT_EQ(i, m_ring_buffer->item_count());
        const size_t index = (3 + i) % 5;
        ASSERT_EQ(i, *m_ring_buffer->get(i - 1));
        ASSERT_EQ(i, *m_ring_buffer->get_abs(index));
    }
}


TEST_F(TestRingBuffer, pop2) { // NOLINT(readability-function-cognitive-complexity)
    populate();
    m_ring_buffer->push(6);
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(i + 2, m_ring_buffer->pop());
        ASSERT_EQ(0, m_ring_buffer->head_index());

        if (i == 4) {
            ASSERT_EQ(0, m_ring_buffer->tail_index());
        } else if (i > 2) {
            ASSERT_EQ(i - 3, m_ring_buffer->tail_index());
        } else {
            ASSERT_EQ(i + 2, m_ring_buffer->tail_index());
        }
        ASSERT_EQ(5 - i - 1, m_ring_buffer->item_count());
    }
    ASSERT_EQ(0, m_ring_buffer->head_index());
    ASSERT_EQ(0, m_ring_buffer->tail_index());
    ASSERT_EQ(0, m_ring_buffer->item_count());
}

} // namespace
