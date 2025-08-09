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

#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <cstdlib>

template<typename T>
class ring_buffer {
public:
    ring_buffer(const size_t size) : m_size(size) {
        m_ring_buffer = new T[size];
    }

    ~ring_buffer() {
        delete[] m_ring_buffer;
    }

    /**
     * Push new data to buffer
     *
     * @param data data to push
     */
    void push(const T &data) {
        if (m_item_count < m_size) {
            m_item_count++;
        }
        if (m_item_count > 1) {
            m_head = (m_head + 1) % m_size;

            if (m_head == m_tail) {
                m_tail = (m_tail + 1) % m_size;
            }
        }
        m_ring_buffer[m_head] = data;
    }

    /**
     * Get data from the buffer
     *
     * @param index relative index from tail to head
     * @return T pointer
     */
    T *get(const size_t index) const {
        size_t abs_index = (m_tail + index) % m_size;

        if (!bounds_check(abs_index)) {
            return nullptr;
        }

        return &m_ring_buffer[abs_index];
    }

    /**
     * Get data from the buffer
     *
     * @param index absolute unchanging index
     * @return T pointer
     */
    T *get_abs(const size_t index) const {
        if (!bounds_check(index)) {
            return nullptr;
        }

        return &m_ring_buffer[index];
    }

    /**
     * Get data from the buffer relative to head
     *
     * @param index index
     * @return T pointer
     */
    T *get_from_head(const size_t index) const {
        size_t abs_index;

        if (m_item_count == 0) {
            return nullptr;
        }

        const size_t norm_index = (index % m_item_count);

        if (norm_index > m_head) {
            abs_index = m_item_count - norm_index + m_head;
        } else {
            abs_index = m_head - norm_index;
        }

        return &m_ring_buffer[abs_index];
    }

    /**
     * Get latest data
     *
     * @return latest data
     */
    inline T *head() const {
        if (m_item_count == 0) {
            return nullptr;
        }

        return &m_ring_buffer[m_head];
    }

    /**
     * Get head index
     *
     * @return head absolute index
     *
     */
    inline size_t head_index() const {
        return m_head;
    }

    /**
     * Get oldest data
     *
     * @return oldest data
     */
    inline T *tail() const {
        if (m_item_count == 0) {
            return nullptr;
        }
        return &m_ring_buffer[m_tail];
    }

    /**
     * Get tail index
     *
     * @return head absolute index
     *
     */
    inline size_t tail_index() const {
        return m_tail;
    }

    /**
     * Remove data from tail
     *
     */
    inline T pop() {
        if (m_item_count == 0) {
            return {};
        }
        m_item_count--;
        size_t old_tail = m_tail;
        if (m_tail != m_head) {
            m_tail = (m_tail + 1) % m_size;
        }
        return m_ring_buffer[old_tail];
    }

    /**
     * Clear all items
     */
    inline void clear() {
        m_head = 0;
        m_tail = 0;
        m_item_count = 0;
    }

    inline size_t capacity() const {
        return m_size;
    }

    inline size_t item_count() const {
        return m_item_count;
    }


private:
    const size_t m_size;
    T *m_ring_buffer;

    // Ring buffer variables
    size_t m_head = 0;
    size_t m_tail = 0;
    size_t m_item_count = 0;

    inline bool bounds_check(size_t abs_index) const {
        if (m_item_count == 0) {
            return false;
        }
        if (m_head > m_tail) {
            if (abs_index < m_tail || abs_index > m_head) {
                return false;
            }
        } else {
            if (abs_index < m_tail && abs_index > m_head) {
                return false;
            }
        }
        return true;
    }
};

#endif
