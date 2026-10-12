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

#ifndef FRAME_DATA_ANALYSER_HPP
#define FRAME_DATA_ANALYSER_HPP

#include "ring_buffer.hpp"

#include "frame_data_defs.hpp"
#include "game_state_reader.h"


class EventListener {
public:
    virtual ~EventListener() = default;
    EventListener() = default;
    EventListener(const EventListener &) = default;
    EventListener(EventListener &&) = default;
    EventListener &operator=(const EventListener &) = default;
    EventListener &operator=(EventListener &&) = default;

    virtual void frame_data(FrameDataPoint frame_data) = 0;
    virtual void distance(float distance) = 0;
    virtual void status(PlayerState status) = 0;
    virtual void game_hooked() = 0;
};

class FrameDataAnalyser {
public:
    FrameDataAnalyser() = delete;
    ~FrameDataAnalyser() = delete;

    FrameDataAnalyser(const FrameDataAnalyser &) = default;
    FrameDataAnalyser(FrameDataAnalyser &&) = delete;
    FrameDataAnalyser &operator=(const FrameDataAnalyser &) = default;
    FrameDataAnalyser &operator=(FrameDataAnalyser &&) = delete;

    /**
     * Hook-up to game's memory and start analysing frames
     *
     * @param callback callback pointer
     */
    static bool start(EventListener *listener);
    /**
     * Stop the analyser loop
     */
    static void stop();
    static bool should_stop();

    static const char *player_status(const PlayerState state);
    static void set_logging(const bool enabled);

private:
    static volatile bool m_stop;
    static RingBuffer<GameFrame> m_frame_buffer;
    static EventListener *m_listener;
    static int m_last_player_intent;
    static bool m_logging;

    // Analysis state
    static RingBuffer<StartFrame> m_p1_start_frames;
    static RingBuffer<StartFrame> m_p2_start_frames;
    // String connection frames
    static RingBuffer<GameFrame> m_p1_str_connection_frames;
    static RingBuffer<GameFrame> m_p2_str_connection_frames;
    // String end frames
    static RingBuffer<GameFrame> m_p1_str_end_frames;
    static RingBuffer<GameFrame> m_p2_str_end_frames;
    // String type frames
    static RingBuffer<GameFrame> m_p1_str_type_frames;
    static RingBuffer<GameFrame> m_p2_str_type_frames;


    static bool init(EventListener *listener);
    static bool loop();

    inline static void log_frame();
    inline static bool flip_player_data(GameFrame &state);
    inline static bool is_attack(const PlayerIntent &intent);
    inline static bool recovery_reset(const PlayerFrame *const previous, const PlayerFrame *const current);
    static const GameFrame *get_game_frame(const uint32_t game_frame);
    static StartFrame get_startup_frame(const GameFrame *const frame, const bool p2, const bool pop);

    inline static bool initiated_attack(const PlayerFrame *const previous, const PlayerFrame *const current);
    static void analyse_start_frames();
    static bool update_game_state();
    static ConnectionEvent has_new_connection();
    inline static bool player_in_stasis(const PlayerFrame *const player_frame);
    inline static bool string_is_active(const PlayerFrame *const player_frame);
    inline static bool is_knockdown(const PlayerFrame *const player_frame);
    inline static bool has_string_startup(const bool p2);
    inline static bool should_handle_string(const PlayerFrame *const player_frame, const bool p2);
    inline static bool string_has_ended_state(const PlayerFrame *const player_frame);
    inline static void reset_string_sm();
    inline static void push_string_type(const GameFrame *const frame, const bool p2);
    inline static bool is_multihit_attack(const PlayerFrame *const player);
    static bool string_is_multihit_attack(RingBuffer<GameFrame> *const player_connections, const bool p2);
    static bool calculate_multihit_string(RingBuffer<GameFrame> *const player_connections, const bool p2);
    static bool calculate_natural_string(RingBuffer<GameFrame> *const player_connections, const bool p2);
    static bool calculate_strings(const bool p2);
    static bool string_has_concluded(const GameFrame *const current, const bool p2);
    static void calculate_single_attack(const ConnectionEvent connection,
                                        const GameFrame *const previous,
                                        const GameFrame *const current);
    static void handle_connection();
    static void handle_strings();
    static void handle_distance();
    static void handle_status();

    static float calculate_distance(const GameFrame *const state);
};

#endif
