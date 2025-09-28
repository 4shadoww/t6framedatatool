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


#include "gui_functions.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include "GLFW/glfw3native.h"

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "logging.h"

bool platform_make_overlay(GLFWwindow *window) {
    Display *display = glfwGetX11Display();
    if (display == nullptr) {
        log_error("failed to get X11 display");
        return false;
    }

    const Window x11_window = glfwGetX11Window(window);
    if (x11_window == 0) {
        log_error("failed to get X11 window");
        return false;
    }

    // Set override_redirect true to make WM not manage this window
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(display, x11_window, CWOverrideRedirect, &attrs);
    XMapWindow(display, x11_window);

    return true;
}
