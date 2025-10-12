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

// DANGER: this is vibe coded

#include "platform_gui.hpp"

#include <algorithm>
#include <string>

#include <windows.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include "gui_constants.hpp"
#include "logging.h"

#define GAME_WINDOW_NAME L"tekken 6"

namespace {
HWND g_window;
HWND g_game_window;
HWINEVENTHOOK g_h_win_event_hook;

VOID CALLBACK WinEventProc(HWINEVENTHOOK /*hWinEventHook*/,
                           DWORD event,
                           HWND hwnd,
                           LONG /*idObject*/,
                           LONG /*idChild*/,
                           DWORD /*dwEventThread*/,
                           DWORD /*dwmsEventTime*/) {
    if (hwnd != g_game_window || event != EVENT_OBJECT_LOCATIONCHANGE) {
        return;
    }
    RECT rect;
    if (GetWindowRect(g_game_window, &rect) == 0) {
        return;
    }

    platform_update_ui_position(rect.left, rect.top, rect.bottom - rect.top, true);
}

bool window_match(const wchar_t *window_name) {
    std::wstring window_name_lower(window_name);
    std::ranges::transform(window_name_lower, window_name_lower.begin(), ::towlower);
    return wcsstr(window_name_lower.c_str(), GAME_WINDOW_NAME) != nullptr;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM /*lParam*/) {
    wchar_t class_name[256];
    wchar_t window_name[256];

    GetClassNameW(hwnd, class_name, sizeof(class_name) / sizeof(wchar_t));
    GetWindowTextW(hwnd, window_name, sizeof(window_name) / sizeof(wchar_t));

    if (!window_match(window_name)) {
        return TRUE;
    }

    log_info("found game window %ls, %ls", class_name, window_name);
    g_game_window = hwnd;
    return FALSE;
}

bool setup_hook() {
    CoInitialize(nullptr);
    g_h_win_event_hook = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE,
                                         EVENT_OBJECT_LOCATIONCHANGE,
                                         nullptr,
                                         WinEventProc,
                                         0,
                                         0,
                                         WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    return g_h_win_event_hook != nullptr;
}

} // namespace

bool platform_make_overlay(GLFWwindow *window) {
    g_window = glfwGetWin32Window(window);
    if (g_window == nullptr) {
        log_error("failed to get Win32 window");
        return false;
    }

    SetWindowLongPtr(g_window, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);

    // Window styles
    LONG style = GetWindowLong(g_window, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU); // NOLINT
    SetWindowLong(g_window, GWL_STYLE, style);

    SetLayeredWindowAttributes(g_window, 0, 255, LWA_ALPHA);

    // Events need to be subscribed here because Windblows doesn't allow
    // another thread to subscribe events and other one to consume
    setup_hook();

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

    SetWindowPos(g_window, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}

void platform_find_game_window() {
    EnumWindows(EnumWindowsProc, 0);
    if (g_game_window == nullptr) {
        log_error("no game window found");
        return;
    }

    // Get initial position
    RECT rect;
    if (GetWindowRect(g_game_window, &rect) == 0) {
        return;
    }

    platform_update_ui_position(rect.left, rect.top, rect.bottom - rect.top, true);
}

void platform_update() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
