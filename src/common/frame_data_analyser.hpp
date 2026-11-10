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
    virtual void input(InputState input) = 0;
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

    /**
     * Reset analysis state and attach a listener for replay
     *
     * @param listener event listener
     */
    static void reset(EventListener *listener);
    /**
     * Push and analyse a frame
     *
     * @param state game frame
     */
    static bool process_frame(GameFrame state);
    /**
     * Set the local side and analyse a recorded frame
     *
     * @param state game frame
     * @param side_val recorded player side
     */
    static bool process_frame(GameFrame state, const int32_t side_val);

    static const char *player_status(const PlayerState state);
    static void set_logging(const bool enabled);

private:
    static volatile bool m_stop;
    static RingBuffer<GameFrame> m_frame_buffer;
    static EventListener *m_listener;
    static int m_last_player_intent;
    static bool m_logging;
    static PlayerSide m_player_side;

    // Analysis state
    static AttackChain m_p1_chain;
    static AttackChain m_p2_chain;

    static bool init(EventListener *listener);
    static bool loop();

    inline static void log_frame();

    static bool update_game_state();
    static void handle_input(const uint32_t game_frame);
    inline static bool is_knockdown(const PlayerFrame *const player_frame);
    static void track_chain(const PlayerFrame *const previous,
                            const PlayerFrame *const current,
                            AttackChain *const chain,
                            const uint32_t game_frame);
    static void track_chains();
    static bool new_connection(const PlayerFrame *const previous, const PlayerFrame *const current);
    static bool left_before_hitbox(const PlayerFrame *const previous,
                                   const PlayerFrame *const current,
                                   const AttackChain *const chain,
                                   const bool auto_string,
                                   const uint32_t game_frame);
    static void report_replaced_string(const PlayerFrame *const previous,
                                       const PlayerFrame *const current,
                                       AttackChain *const chain,
                                       const bool auto_string);
    static void report_chain(AttackChain *const chain, const bool p2, const uint32_t recovery);
    static void calculate_connection(const GameFrame *const previous, const GameFrame *const current, const bool p2);
    static void handle_connection();
    static void handle_whiff(const PlayerFrame *const attacker, AttackChain *const chain, const bool p2);
    static void handle_whiffs();
    static void handle_distance();
    static void handle_status();

    static float calculate_distance(const GameFrame *const state);
};

#endif
