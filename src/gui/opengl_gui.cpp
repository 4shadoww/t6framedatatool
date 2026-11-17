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

#include "arg_parser.hpp"
#include "color_scheme.hpp"
#include "event_bus.hpp"
#include "frame_data_analyser.hpp"
#include "frame_dumper.h"
#include "game_state_reader.h"
#include "gui_constants.hpp"
#include "input_dumper.h"
#include "logging.h"
#include "platform_gui.hpp"
#include "platform_threading.hpp"
#include "version.hpp"

#include "component/overlay_canvas.hpp"

#include "view/command_history_view.hpp"
#include "view/configuration_button_view.hpp"
#include "view/configuration_view.hpp"
#include "view/frame_data_view.hpp"
#include "viewmodel/command_history_view_model.hpp"
#include "viewmodel/configuration_view_model.hpp"
#include "viewmodel/frame_data_view_model.hpp"

namespace {

class GuiEventReceiver {
public:
    GuiEventReceiver() noexcept {
        EventBus::subscribe<GameHookedEvent>(this);
    }

    static void on_event(const GameHookedEvent & /*event*/) {
        platform_find_game_window();
    }
};

struct {
    FrameDataViewModel frame_data;
    CommandHistoryViewModel command_history;
    ConfigurationViewModel configuration;
} view_models;

GuiEventReceiver gui_event_receiver;

void setup_style() {
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = ui::em(0.32F);
    style.FrameRounding = ui::em(0.24F);
    style.ChildRounding = ui::em(0.24F);
    style.PopupRounding = ui::em(0.24F);
    style.ScrollbarRounding = ui::em(0.24F);
    style.GrabRounding = ui::em(0.24F);
    style.WindowPadding = ui::em(0.48F, 0.40F);
    style.FramePadding = ui::em(0.32F, 0.20F);
    style.ItemSpacing = ui::em(0.32F, 0.24F);
    style.WindowBorderSize = ui::hairline();

    ImVec4 *const colors = style.Colors;
    colors[ImGuiCol_WindowBg] = color_scheme::WINDOW_BACKGROUND;
    colors[ImGuiCol_ChildBg] = color_scheme::CHILD_BACKGROUND;
    colors[ImGuiCol_FrameBg] = color_scheme::FRAME_BACKGROUND;
    colors[ImGuiCol_Border] = color_scheme::BORDER;
    colors[ImGuiCol_Separator] = color_scheme::SEPARATOR;
    colors[ImGuiCol_Text] = color_scheme::TEXT;
    colors[ImGuiCol_TextDisabled] = color_scheme::MUTED_TEXT;
}

void draw_overlay() {
    const ImVec2 canvas = OverlayCanvas::size();
    FrameDataView::draw(canvas, view_models.frame_data);
    CommandHistoryView::draw(canvas, view_models.command_history);
    ConfigurationButtonView::draw(canvas, view_models.configuration);
    ConfigurationView::draw(canvas, view_models.configuration);
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
        if (FrameDataAnalyser::start()) {
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

void set_mouse_passthrough(GLFWwindow *const window, bool &passthrough, const bool enabled) {
    if (passthrough == enabled) {
        return;
    }

    glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, enabled ? GLFW_TRUE : GLFW_FALSE);
    passthrough = enabled;
}

void gui_loop(GLFWwindow *const window) {
    bool passthrough = true;

    while (glfwWindowShouldClose(window) == 0) {
        const ImVec2 canvas = OverlayCanvas::size();
        double mouse_x = 0.0;
        double mouse_y = 0.0;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        const bool over_button =
            ConfigurationButtonView::contains(canvas, ImVec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y)));
        const bool interactive = view_models.configuration.is_open() || over_button;
        set_mouse_passthrough(window, passthrough, !interactive);

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
    input_dump_stop();
    frame_dump_stop();
}

void apply_config(const Configuration &config) {
    log_set_level(config.log_level);
    FrameDataAnalyser::set_logging(config.frame_data_logging);
    set_memory_reader_logging(config.game_struct_logging);

    if (config.frame_dump_path != nullptr) {
        (void) frame_dump_start(config.frame_dump_path);
    }
    if (config.input_dump_path != nullptr) {
        (void) input_dump_start(config.input_dump_path);
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
        input_dump_stop();
        frame_dump_stop();
        return -1;
    }

    start_gui(window);

    return 0;
}
