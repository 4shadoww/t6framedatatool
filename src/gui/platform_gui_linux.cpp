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


#include "platform_gui.hpp"

#include <cstring>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include "GLFW/glfw3native.h"

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "gui_constants.hpp"
#include "logging.h"

namespace {
Window g_window;
Window g_game_window;
Display *g_display;
Display *g_event_display;

char *read_string_property(const Window window, const char *property_name, unsigned long *len) {
    const Atom prop = XInternAtom(g_display, property_name, False);
    Atom type = 0;
    int form = 0;
    unsigned long remain = 0;
    unsigned char *list = nullptr;

    if (XGetWindowProperty(g_display, window, prop, 0, 1024, False, XA_STRING, &type, &form, len, &remain, &list) !=
        Success) {
        log_error("failed to read window property %s", property_name);
        *len = 0;
        return nullptr;
    }

    return (char *) list; // NOLINT
}

// WM_CLASS is two NUL-separated strings (instance, then class). AppImage sets
// instance to "AppRun.wrapped" and class to "RPCS3", so both have to be searched.
bool property_contains(const char *value, const unsigned long len, const char *needle) {
    if (value == nullptr || needle == nullptr || len == 0) {
        return false;
    }

    const char *cursor = value;
    const char *end = value + len;
    while (cursor < end) {
        if (*cursor != '\0' && strcasestr(cursor, needle) != nullptr) {
            return true;
        }

        while (cursor < end && *cursor != '\0') {
            cursor++;
        }
        if (cursor < end) {
            cursor++;
        }
    }

    return false;
}

Window *get_windows(unsigned long *len) {
    const Atom prop = XInternAtom(g_display, "_NET_CLIENT_LIST", False);
    Atom type = 0;
    int form = 0;
    unsigned long remain = 0;
    unsigned char *list = nullptr;

    if (XGetWindowProperty(g_display,
                           XDefaultRootWindow(g_display),
                           prop,
                           0,
                           1024,
                           False,
                           XA_WINDOW,
                           &type,
                           &form,
                           len,
                           &remain,
                           &list) != Success) {
        log_error("failed to get window list");
        return nullptr;
    }
    return (Window *) list; // NOLINT
}
bool window_match(const char *window_class,
                  const unsigned long class_len,
                  const char *window_name,
                  const unsigned long name_len) {
    if (!property_contains(window_class, class_len, RPSC3_CLASS)) {
        return false;
    }

    if (!property_contains(window_name, name_len, RPSC3_NAME)) {
        return false;
    }

    return true;
}
} // namespace

bool platform_make_overlay(GLFWwindow *window) {
    g_display = glfwGetX11Display();
    if (g_display == nullptr) {
        log_error("failed to get GLFW X11 display");
        return false;
    }

    g_event_display = XOpenDisplay(nullptr);

    if (g_event_display == nullptr) {
        log_error("failed to get display X11 display");
        return false;
    }

    g_window = glfwGetX11Window(window);
    if (g_window == 0) {
        log_error("failed to get X11 window");
        return false;
    }

    // Event mask
    XSelectInput(g_display, g_window, ExposureMask);

    // Set override_redirect true to make WM not manage this window
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(g_display, g_window, CWOverrideRedirect, &attrs);
    XMapWindow(g_display, g_window);

    return true;
}

void platform_update_ui_position(const int game_x, const int game_y, const int game_height, const bool margin) {
    int x = 0;
    int y = 0;
    if (margin) {
        x = game_x + BORDER_MARGIN_X;
        y = game_y - BORDER_MARGIN_Y + game_height - MAX_HEIGTH;
    } else {
        x = game_x;
        y = game_y + game_height - MAX_HEIGTH;
    }

    XMoveWindow(g_display, g_window, x, y);
    XMapRaised(g_display, g_window);
}

void platform_find_game_window() {
    unsigned long len = 0;

    Window *windows = get_windows(&len);

    if (windows == nullptr) {
        return;
    }

    bool found = false;
    for (unsigned long i = 0; i < len; i++) {
        unsigned long class_len = 0;
        unsigned long name_len = 0;
        char *window_class = read_string_property(windows[i], "WM_CLASS", &class_len);
        char *window_name = read_string_property(windows[i], "WM_NAME", &name_len);
        if (window_match(window_class, class_len, window_name, name_len)) {
            log_info("found game window %s, %s", window_class, window_name);
            g_game_window = windows[i];
            found = true;
        }

        if (window_class != nullptr) {
            XFree(window_class);
        }
        if (window_name != nullptr) {
            XFree(window_name);
        }
        if (found) {
            break;
        }
    }

    XFree(windows);

    if (!found) {
        log_error("no game window found");
        return;
    }

    XSelectInput(g_event_display, g_game_window, StructureNotifyMask);

    XWindowAttributes xwa;
    XGetWindowAttributes(g_display, g_game_window, &xwa);
    platform_update_ui_position(xwa.x, xwa.y, xwa.height, true);
}

void platform_update() {
    while (XPending(g_event_display) > 0) {
        XEvent e;
        XNextEvent(g_event_display, &e);

        if (e.type == ConfigureNotify && e.xconfigure.window == g_game_window) {
            const XConfigureEvent xce = e.xconfigure;
            platform_update_ui_position(xce.x, xce.y, xce.height, true);
        }
    }
}
