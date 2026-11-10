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
#include "frame_data_analyser.hpp"
#include "frame_dumper.h"
#include "game_state_reader.h"
#include "gui_constants.hpp"
#include "logging.h"
#include "platform_gui.hpp"
#include "platform_threading.hpp"
#include "version.hpp"

namespace {

#define RESET_UI_MILLIS 2500

constexpr float OVERLAY_FONT_SIZE = 20.0F;
constexpr float LABEL_COLUMN_WIDTH = 210.0F;
constexpr ImVec4 POSITIVE_COLOR = ImVec4(0.35F, 1.0F, 0.55F, 1.0F);
constexpr ImVec4 NEGATIVE_COLOR = ImVec4(1.0F, 0.35F, 0.35F, 1.0F);
constexpr ImVec4 KNOCKDOWN_COLOR = ImVec4(1.0F, 0.75F, 0.25F, 1.0F);
constexpr ImVec4 MUTED_COLOR = ImVec4(0.70F, 0.72F, 0.78F, 1.0F);

std::chrono::steady_clock::rep g_last_update = 0;
FrameDataPoint g_data_point = {};
float g_distance = 0;
PlayerState g_status = {};
bool g_game_hooked = false;

class Listener : public EventListener {
public:
    void frame_data(const FrameDataPoint frame_data) override;
    void distance(const float distance) override;
    void status(const PlayerState state) override;
    void input(const InputState input) override;
    void game_hooked() override;
};

void Listener::frame_data(const FrameDataPoint frame_data) {
    g_last_update = std::chrono::steady_clock::now().time_since_epoch().count();
    g_data_point = frame_data;

    const PlayerFrameData &local = frame_data.player_side == PlayerSide::LEFT ? frame_data.p1 : frame_data.p2;
    log_info("local player: P%d, startup frames: %d (%d~%d), frame advantage: %d, KD: %d",
             frame_data.player_side == PlayerSide::LEFT ? 1 : 2,
             local.startup_frames,
             local.first_active_frame,
             local.last_active_frame,
             local.frame_advantage,
             local.knock_down);
}

void Listener::distance(const float distance) {
    g_distance = distance;
}

void Listener::status(const PlayerState state) {
    g_status = state;
}

void Listener::input(const InputState /*input*/) {
    // NOP
}

void Listener::game_hooked() {
    g_game_hooked = true;
    platform_find_game_window();
}

Listener g_listener;

bool has_recent_frame_data() {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = now.time_since_epoch() - std::chrono::steady_clock::duration(g_last_update);
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    return millis <= RESET_UI_MILLIS;
}

const PlayerFrameData &local_player_data() {
    return g_data_point.player_side == PlayerSide::LEFT ? g_data_point.p1 : g_data_point.p2;
}

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
    style.WindowBorderSize = 0.0F;

    ImVec4 *const colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.04F, 0.05F, 0.07F, 0.92F);
    colors[ImGuiCol_ChildBg] = ImVec4(0.07F, 0.08F, 0.11F, 0.95F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.11F, 0.13F, 0.17F, 1.0F);
    colors[ImGuiCol_Border] = ImVec4(0.18F, 0.22F, 0.28F, 1.0F);
    colors[ImGuiCol_Separator] = ImVec4(0.18F, 0.22F, 0.28F, 1.0F);
    colors[ImGuiCol_Text] = ImVec4(0.92F, 0.94F, 0.97F, 1.0F);
    colors[ImGuiCol_TextDisabled] = MUTED_COLOR;
}

void draw_advantage(const int advantage) {
    ImGui::TextColored(advantage < 0 ? NEGATIVE_COLOR : POSITIVE_COLOR, "%+d", advantage);
}

void draw_label(const char *const label) {
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine(LABEL_COLUMN_WIDTH);
}

void draw_no_game() {
    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    draw_label("Startup Frames");
    ImGui::TextUnformatted("--- (No game hooked)");

    draw_label("Frame Advantage");
    ImGui::TextUnformatted("---");

    draw_label("Status");
    ImGui::TextUnformatted("---");

    draw_label("Distance");
    ImGui::TextUnformatted("---");

    ImGui::PopFont();
}

void draw_no_frame_data() {
    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    draw_label("Startup Frames");
    ImGui::TextUnformatted("---");

    draw_label("Frame Advantage");
    ImGui::TextUnformatted("---");

    draw_label("Status");
    ImGui::TextUnformatted(FrameDataAnalyser::player_status(g_status));

    draw_label("Distance");
    ImGui::Text("%.2f", g_distance);

    ImGui::PopFont();
}

void draw_frame_data() {
    ImGui::PushFont(nullptr, OVERLAY_FONT_SIZE);

    const PlayerFrameData &data = local_player_data();
    const bool knock_down = g_data_point.p1.knock_down || g_data_point.p2.knock_down;

    draw_label("Startup Frames");
    ImGui::Text("%d (%u~%u)", data.startup_frames, data.first_active_frame, data.last_active_frame);

    draw_label("Frame Advantage");
    if (knock_down) {
        ImGui::TextColored(KNOCKDOWN_COLOR, "KD");
    } else {
        draw_advantage(data.frame_advantage);
    }

    draw_label("Status");
    ImGui::TextUnformatted(FrameDataAnalyser::player_status(g_status));

    draw_label("Distance");
    ImGui::Text("%.2f", g_distance);

    ImGui::PopFont();
}

void draw_overlay() {
    const ImGuiViewport *const viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    const unsigned int flag_bits = static_cast<unsigned int>(ImGuiWindowFlags_NoDecoration) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoMove) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoResize) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoSavedSettings) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoBringToFrontOnFocus) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoNavFocus) |
                                   static_cast<unsigned int>(ImGuiWindowFlags_NoScrollbar);
    const auto flags = static_cast<ImGuiWindowFlags>(flag_bits);

    ImGui::Begin("##overlay", nullptr, flags);

    if (!g_game_hooked) {
        draw_no_game();
    } else if (!has_recent_frame_data()) {
        draw_no_frame_data();
    } else {
        draw_frame_data();
    }

    ImGui::End();
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
    platform_update_ui_position(0, 0, MAX_HEIGTH, false);
    g_game_hooked = false;
}

void analyser_loop() {
    while (true) {
        if (FrameDataAnalyser::start(&g_listener)) {
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
