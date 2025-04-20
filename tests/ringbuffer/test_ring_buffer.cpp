#include <gtest/gtest.h>

#include "ring_buffer.hpp"

#define BUFFER_SIZE 5

class test_ring_buffer : public testing::Test {
protected:
    ring_buffer<int> *m_ring_buffer;

    test_ring_buffer(){}

    ~test_ring_buffer() override {}

    void SetUp() override {
        m_ring_buffer = new ring_buffer<int>(BUFFER_SIZE);
    }

    void TearDown() override {
        delete m_ring_buffer;
    }

    void populate();
    void pop_all();
};

void test_ring_buffer::populate() {
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

void test_ring_buffer::pop_all() {
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

TEST_F(test_ring_buffer, initial_state) {
    ASSERT_EQ(0, m_ring_buffer->head_index());
    ASSERT_EQ(0, m_ring_buffer->tail_index());
    ASSERT_EQ(0, m_ring_buffer->item_count());
    EXPECT_EQ(BUFFER_SIZE, m_ring_buffer->capacity());
}

TEST_F(test_ring_buffer, push_full) {
    populate();
    ASSERT_EQ(5, m_ring_buffer->item_count());
    ASSERT_EQ(4, m_ring_buffer->head_index());
    ASSERT_EQ(0, m_ring_buffer->tail_index());
}

TEST_F(test_ring_buffer, push_overfill) {
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

TEST_F(test_ring_buffer, get_tail_head) {
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

TEST_F(test_ring_buffer, empty_get_bounds) {
    ASSERT_EQ(nullptr, m_ring_buffer->get(0));
    ASSERT_EQ(nullptr, m_ring_buffer->get(4));
    ASSERT_EQ(nullptr, m_ring_buffer->get_abs(0));
    ASSERT_EQ(nullptr, m_ring_buffer->get_abs(4));
    ASSERT_EQ(nullptr, m_ring_buffer->get_from_head(0));
    ASSERT_EQ(nullptr, m_ring_buffer->get_from_head(4));
    ASSERT_EQ(nullptr, m_ring_buffer->tail());
    ASSERT_EQ(nullptr, m_ring_buffer->head());
}

TEST_F(test_ring_buffer, get_bounds) {
    populate();
    // When has data, cannot go out-of-bounds
    ASSERT_NE(nullptr, m_ring_buffer->get(0));
    ASSERT_NE(nullptr, m_ring_buffer->get(BUFFER_SIZE));
    ASSERT_NE(nullptr, m_ring_buffer->get_from_head(0));
    ASSERT_NE(nullptr, m_ring_buffer->get_from_head(BUFFER_SIZE));

    ASSERT_NE(nullptr, m_ring_buffer->get_abs(0));
    ASSERT_EQ(nullptr, m_ring_buffer->get_abs(BUFFER_SIZE));
}

TEST_F(test_ring_buffer, clear) {
    populate();
    m_ring_buffer->clear();

    ASSERT_EQ(0, m_ring_buffer->head_index());
    ASSERT_EQ(0, m_ring_buffer->tail_index());
    ASSERT_EQ(0, m_ring_buffer->item_count());
}

TEST_F(test_ring_buffer, pop) {
    populate();
    pop_all();
}

TEST_F(test_ring_buffer, repopulate) {
    populate();
    pop_all();

    for (int i = 1; i < 6; i++) {
        m_ring_buffer->push(i);
        ASSERT_EQ(i, m_ring_buffer->item_count());
        size_t index = (3 + i) % 5;
        ASSERT_EQ(i, *m_ring_buffer->get(i - 1));
        ASSERT_EQ(i, *m_ring_buffer->get_abs(index));
    }
}


TEST_F(test_ring_buffer, pop2) {
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
