/*
  Copyright (C) 2025-2026 Noa-Emil Nissinen

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

#include <chrono>
#include <thread>

#include <glad/gl.h> // NOLINT

#include <GLFW/glfw3.h>

#include <imgui.h> // NOLINT
#include <imgui_impl_glfw.h> // NOLINT
#include <imgui_impl_opengl3.h> // NOLINT

#include "analyser_event_router.hpp"
#include "arg_parser.hpp"
#include "frame_data_analyser.hpp"
#include "frame_dumper.h"
#include "game_state_reader.h"
#include "gui_constants.hpp"
#include "logging.h"
#include "platform_gui.hpp"
#include "platform_threading.hpp"
#include "version.hpp"

#include "component/overlay_canvas.hpp"

#include "view/command_history_view.hpp"
#include "view/frame_data_view.hpp"
#include "viewmodel/frame_data_view_model.hpp"

namespace {

constexpr ImVec4 MUTED_COLOR = ImVec4(0.65F, 0.65F, 0.65F, 1.0F);

struct {
    FrameDataViewModel frame_data;
} view_models;

AnalyserEventRouter g_event_router(&view_models.frame_data);

void setup_style() {
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 8.0F;
    style.FrameRounding = 6.0F;
    style.ChildRounding = 6.0F;
    style.PopupRounding = 6.0F;
    style.ScrollbarRounding = 6.0F;
    style.GrabRounding = 6.0F;
    style.WindowPadding = ImVec2(12.0F, 10.0F);
    style.FramePadding = ImVec2(8.0F, 5.0F);
    style.ItemSpacing = ImVec2(8.0F, 6.0F);
    style.WindowBorderSize = 1.0F;

    ImVec4 *const colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.02F, 0.02F, 0.02F, 0.92F);
    colors[ImGuiCol_ChildBg] = ImVec4(0.04F, 0.04F, 0.04F, 0.95F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.08F, 0.08F, 0.08F, 1.0F);
    colors[ImGuiCol_Border] = ImVec4(0.16F, 0.16F, 0.16F, 1.0F);
    colors[ImGuiCol_Separator] = ImVec4(0.16F, 0.16F, 0.16F, 1.0F);
    colors[ImGuiCol_Text] = ImVec4(0.92F, 0.92F, 0.92F, 1.0F);
    colors[ImGuiCol_TextDisabled] = MUTED_COLOR;
}

void draw_overlay() {
    const ImVec2 canvas = OverlayCanvas::size();
    FrameDataPanel::draw(canvas, view_models.frame_data);
    CommandHistoryView::draw(canvas);
}

GLFWwindow *create_window() {
    GLFWwindow *window = nullptr;
    if (glfwInit() == 0) {
        return nullptr;
    }

    // Window hints
    // OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Overlay
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(MAX_WIDTH, MAX_HEIGTH, WINDOW_NAME, nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return nullptr;
    }

    return window;
}

bool setup_graphics(GLFWwindow *const window) {
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        log_error("failed to load glad GL");
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    setup_style();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        log_error("failed to initialize imgui GLFW backend");
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        log_error("failed to initialize imgui OpenGL backend");
        return false;
    }

    return true;
}

GLFWwindow *setup_gui() {
    GLFWwindow *const window = create_window();
    if (window == nullptr) {
        log_error("failed to create a window");
        return nullptr;
    }

    if (!platform_make_overlay(window)) {
        log_error("failed to create tool overlay");
        return nullptr;
    }

    glfwSetWindowPos(window, 0, 0);
    glfwShowWindow(window);
    glfwMakeContextCurrent(window);

    if (!setup_graphics(window)) {
        log_error("setting up graphics context failed");
        return nullptr;
    }

    return window;
}

void gui_state_no_game() {
    platform_update_ui_bounds(0, 0, MAX_WIDTH, MAX_HEIGTH);
    view_models.frame_data.on_game_hooked(false);
}

void analyser_loop() {
    while (true) {
        if (FrameDataAnalyser::start(&g_event_router)) {
            break;
        }

        if (FrameDataAnalyser::should_stop()) {
            log_debug("shutting down analyser");
            return;
        }

        log_debug("starting analyser again after timeout");
        gui_state_no_game();
        std::this_thread::sleep_for(std::chrono::seconds(ANALYSER_START_INTERVAL));
    }
}

void gui_loop(GLFWwindow *const window) {
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        platform_update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        draw_overlay();

        ImGui::Render();

        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
        glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void start_gui(GLFWwindow *const window) {
    std::thread analyser_thread(&analyser_loop);
    set_realtime_prio(analyser_thread);

    gui_loop(window);

    // GUI has exited
    FrameDataAnalyser::stop();
    analyser_thread.join();
    frame_dump_stop();
}

void apply_config(const Configuration &config) {
    log_set_level(config.log_level);
    FrameDataAnalyser::set_logging(config.frame_data_logging);
    set_memory_reader_logging(config.game_struct_logging);

    if (config.frame_dump_path != nullptr) {
        (void) frame_dump_start(config.frame_dump_path);
    }
}

} // namespace

int main(const int argc, const char *const *const argv) {
    Configuration config = ArgParser::create_default_config();
    const int result = ArgParser::parse_arguments(argc, argv, &config);
    if (result == 1) {
        return 1;
    } else if (result == -1) {
        // Early exit
        return 0;
    }

    apply_config(config);

    log_info("%s %s", PROGRAM_NAME, VERSION);

    GLFWwindow *const window = setup_gui();
    if (window == nullptr) {
        frame_dump_stop();
        return -1;
    }

    start_gui(window);

    return 0;
}
