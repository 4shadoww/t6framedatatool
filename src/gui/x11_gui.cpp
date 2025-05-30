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

#include <X11/Xutil.h>
#include <chrono>
#include <iostream>
#include <thread>

#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>

#include "logging.h"

#include "frame_data_analyser.hpp"

#define TICK_LENGTH 200
#define ANALYSER_START_INTERVAL 1
#define FONT "-adobe-courier-bold-r-normal-*-*-*-*-*-*-150-*-*"
#define FONT_HEIGH 15
#define MARGIN 5
#define MAX_WIDTH 1000
#define WINDOW_NAME "T6FDTOOL"
#define STAT_LINES 4

// UI
#define NO_STARTUP_FRAMES "Startup frames --- (No game hooked)"
#define NO_FRAME_ADVANTAGE "Frame Advantage ---"
#define NO_STATUS "Status ---"
#define NO_DISTANCE "Distance ---"

#define STARTUP_FRAMES "Startup frames %d"
#define FRAME_ADVANTAGE "Frame Advantage %d"
#define STATUS "Status %s"
#define DISTANCE "Distance %.2f"

struct {
    Display *display;
    Screen *screen;
    int screen_num;
    Window window;

    GC text_gc;
    GC text_gc_red;
    XColor green;
    XColor red;
    XColor green_colour;
    XColor red_colour;

    int x;
    int y;
    int width;
    int height;
    int line_height;
    int total_height;
} g_x11_session;

struct frame_data_point g_data_point = {};
float g_distance = 0;
player_state g_status = {};
bool g_game_hooked = false;

class listener : public event_listener {
public:
    void frame_data(const frame_data_point frame_data) override;
    void distance(const float distance) override;
    void status(const player_state state) override;
    void game_hooked() override;
};

void listener::frame_data(const frame_data_point frame_data) {
    g_data_point = frame_data;
}

void listener::distance(const float distance) {
    g_distance = distance;
}

void listener::status(const player_state state) {
    g_status = state;
}

void new_frame_data(struct frame_data_point data_point) {
    std::cout << "startup frames: " << data_point.startup_frames << ", frame advantage: " <<
                 data_point.frame_advantage << std::endl;
}

void listener::game_hooked() {
    g_game_hooked = true;
}

listener g_listener;


inline bool load_graphics() {
    // Allocate contexes
    g_x11_session.text_gc = XCreateGC(g_x11_session.display, g_x11_session.window, 0, NULL);
    g_x11_session.text_gc_red = XCreateGC(g_x11_session.display, g_x11_session.window, 0, NULL);

    // Colors
    // Red
    g_x11_session.red.red = 65535;
    g_x11_session.red.green = 0;
    g_x11_session.red.blue = 0;
    g_x11_session.red.flags = DoRed | DoGreen | DoBlue;
    XAllocColor(g_x11_session.display, DefaultColormap(g_x11_session.display, g_x11_session.screen_num), &g_x11_session.red);

    // Green
    g_x11_session.green.red = 0;
    g_x11_session.green.green = 65535;
    g_x11_session.green.blue = 0;
    g_x11_session.green.flags = DoRed | DoGreen | DoBlue;
    XAllocColor(g_x11_session.display, DefaultColormap(g_x11_session.display, g_x11_session.screen_num), &g_x11_session.green);

    const XFontStruct* font_info = XLoadQueryFont(g_x11_session.display, FONT);
    if (!font_info) {
        log_error("failed to load font");
        return false;
    }

    XSetFont(g_x11_session.display, g_x11_session.text_gc, font_info->fid);
    XSetForeground(g_x11_session.display, g_x11_session.text_gc, g_x11_session.green.pixel);
    XSetBackground(g_x11_session.display, g_x11_session.text_gc, BlackPixel(g_x11_session.display, g_x11_session.screen_num));

    XSetFont(g_x11_session.display, g_x11_session.text_gc_red, font_info->fid);
    XSetForeground(g_x11_session.display, g_x11_session.text_gc_red, g_x11_session.red.pixel);
    XSetBackground(g_x11_session.display, g_x11_session.text_gc_red, BlackPixel(g_x11_session.display, g_x11_session.screen_num));
    return true;
}

inline void mouse_passtrough() {
    XRectangle rect;
    XserverRegion region = XFixesCreateRegion(g_x11_session.display, &rect, 1);
    XFixesSetWindowShapeRegion(g_x11_session.display, g_x11_session.window, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(g_x11_session.display, region);
}

bool init_gui() {
    g_x11_session.display = XOpenDisplay(NULL);

    if (!g_x11_session.display) {
        log_error("cannot open display");
        return false;
    }

    Window defaultroot = DefaultRootWindow(g_x11_session.display);

    // Session variables
    g_x11_session.screen_num = DefaultScreen(g_x11_session.display);
    g_x11_session.screen = XDefaultScreenOfDisplay(g_x11_session.display);
    g_x11_session.x = 0;
    g_x11_session.y = 0;
    g_x11_session.line_height = FONT_HEIGH + MARGIN;
    g_x11_session.total_height = g_x11_session.line_height * STAT_LINES;
    g_x11_session.width = MAX_WIDTH;
    g_x11_session.height = g_x11_session.total_height;

    // Load visual info
    XVisualInfo vinfo;
    XMatchVisualInfo(g_x11_session.display, g_x11_session.screen_num, 32, TrueColor, &vinfo);

    // Set window attributes
    XSetWindowAttributes attr;
    attr.override_redirect = 1;
    attr.colormap = XCreateColormap(g_x11_session.display, defaultroot, vinfo.visual, AllocNone);
    attr.border_pixel = 0;
    attr.background_pixel = 0;

    // Create window
    g_x11_session.window = XCreateWindow(g_x11_session.display, defaultroot,
                                     g_x11_session.x,
                                     g_x11_session.y,
                                     g_x11_session.width,
                                     g_x11_session.height,
                                     0,
                                     vinfo.depth,
                                     InputOutput,
                                     vinfo.visual,
                                     CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect,
                                     &attr);

    XStoreName(g_x11_session.display, g_x11_session.window, WINDOW_NAME);

    // Map window
    XMapRaised(g_x11_session.display, g_x11_session.window);

    // Set event mask
    XSelectInput(g_x11_session.display, g_x11_session.window, ExposureMask | StructureNotifyMask);

    mouse_passtrough();

    if (!load_graphics()) {
        log_error("failed to init graphics");
        return false;
    }

    XFlush(g_x11_session.display);

    return true;
}

void gui_state_no_game() {
    g_game_hooked = false;
}

void analyser_loop() {
    while (true) {
        if (frame_data_analyser::start(&g_listener)) {
            break;
        }

        log_debug("starting analyser again after timeout");
        gui_state_no_game();
        std::this_thread::sleep_for (std::chrono::seconds(ANALYSER_START_INTERVAL));
    }
}

inline void draw_line(const unsigned int line_num, const char *text, const size_t text_len) {
    XDrawString(g_x11_session.display, g_x11_session.window, g_x11_session.text_gc, 0, g_x11_session.line_height * line_num, text, text_len);
}

inline void draw_line_red(const unsigned int line_num, const char *text, const size_t text_len) {
    XDrawString(g_x11_session.display, g_x11_session.window, g_x11_session.text_gc_red, 0, g_x11_session.line_height * line_num, text, text_len);
}

inline void draw_no_game() {
    draw_line(1, NO_STARTUP_FRAMES, sizeof(NO_STARTUP_FRAMES) - 1);
    draw_line(2, NO_FRAME_ADVANTAGE, sizeof(NO_FRAME_ADVANTAGE) - 1);
    draw_line(3, NO_STATUS, sizeof(NO_STATUS) - 1);
    draw_line(4, NO_DISTANCE, sizeof(NO_DISTANCE) - 1);
}

inline void draw_game_state() {
    char buffer[50];

    int length = sprintf(buffer, STARTUP_FRAMES, g_data_point.startup_frames);
    draw_line(1, buffer, length);


    length = sprintf(buffer, FRAME_ADVANTAGE, g_data_point.frame_advantage);
    if (g_data_point.frame_advantage < 0) {
        draw_line_red(2, buffer, length);
    } else {
        draw_line(2, buffer, length);
    }

    length = sprintf(buffer, STATUS, frame_data_analyser::player_status(g_status));
    draw_line(3, buffer, length);

    length = sprintf(buffer, DISTANCE, g_distance);
    draw_line(4, buffer, length);
}

inline void draw() {
    XEvent e;
    XCheckMaskEvent(g_x11_session.display, ExposureMask | StructureNotifyMask, &e);

    XClearArea(g_x11_session.display,
               g_x11_session.window,
               g_x11_session.x,
               g_x11_session.y,
               g_x11_session.width,
               g_x11_session.height,
               True);

    XFlush(g_x11_session.display);

    if (!g_game_hooked) {
        draw_no_game();
    } else {
        draw_game_state();
    }

    XFlush(g_x11_session.display);
}

void gui_loop() {
    while (true) {
        auto start = std::chrono::high_resolution_clock::now();

        draw();

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

    return 0;
}
