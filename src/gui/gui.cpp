#include <chrono>
#include <iostream>
#include <thread>

#include <xosd.h>

#include "logging.h"

#include "frame_data_analyser.hpp"

#define XOSD_FONT "-adobe-courier-bold-r-normal-*-*-*-*-*-*-150-*-*"
#define TICK_LENGTH 200

xosd *g_osd;
struct frame_data_point g_data_point = {};
float g_distance = 0;

class listener : public event_listener {
public:
    void frame_data(const frame_data_point frame_data) override;
    void distance(const float distance) override;
};

void listener::frame_data(const frame_data_point frame_data) {
    g_data_point = frame_data;
}

void listener::distance(const float distance) {
    g_distance = distance;
}

void new_frame_data(struct frame_data_point data_point) {
    std::cout << "startup frames: " << data_point.startup_frames << ", frame advantage: " <<
                 data_point.frame_advantage << std::endl;
}

listener g_listener;

bool init_gui() {
    g_osd = xosd_create(3);
    if (g_osd == nullptr) {
        log_fatal("failed to create window");
        return false;
    }

    if (xosd_set_font(g_osd, XOSD_FONT)) {
        log_fatal("failed to set font");
        return false;
    }

    if (xosd_set_shadow_offset(g_osd, 1)) {
        log_fatal("failed to set shadow offset");
        return false;
    }


    if (xosd_set_pos(g_osd, XOSD_bottom) == -1) {
        log_fatal("failed to set position");
        return false;
    }

    return true;
}

void analyser_callback(struct frame_data_point data_point) {
    g_data_point = data_point;
    log_info("startup frames: %d, frame advantage: %d", data_point.startup_frames, data_point.frame_advantage);
}

void analyser_loop() {
    frame_data_analyser::start(&g_listener);
}

void gui_loop() {
    while (true) {
        auto start = std::chrono::high_resolution_clock::now();
        if (g_data_point.frame_advantage > -1) {
            if (xosd_set_colour(g_osd, "LawnGreen")) {
                log_fatal("failed to set color");
                return;
            }
        } else {
            if (xosd_set_colour(g_osd, "Red")) {
                log_fatal("failed to set color");
                return;
            }
        }

        // Startup frame
        if (xosd_display(g_osd, 0, XOSD_printf, "Startup Frame %d", g_data_point.startup_frames) == -1) {
            log_fatal("failed to display");
            return;
        }

        // Frame advantage
        if (xosd_display(g_osd, 1, XOSD_printf, "Frame Advantage %d", g_data_point.frame_advantage) == -1) {
            log_fatal("failed to display");
            return;
        }

        // Distance
        if (xosd_display(g_osd, 2, XOSD_printf, "Distance %.2f", g_distance) == -1) {
            log_fatal("failed to display");
            return;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto delta = end - start;
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        std::this_thread::sleep_for (std::chrono::nanoseconds((TICK_LENGTH - millis)));
    }
}

void start_gui() {
    std::thread analyser_thread(&analyser_loop);
    std::thread gui_thread(&gui_loop);

    // GUI has exited
    gui_thread.join();

    frame_data_analyser::stop();
    analyser_thread.join();
}

int main(const int argc, const char **argv) {
    log_set_level(LOG_TRACE);

    if (!init_gui()) {
        log_fatal("failed to init");
        return 1;
    }

    start_gui();

    xosd_destroy(g_osd);

    return 0;
}
