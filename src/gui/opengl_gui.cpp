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

// Based on text rendering example from: https://learnopengl.com

#include <map>
#include <string>
#include <thread>

#include <glad/gl.h> // NOLINT

#include <ft2build.h> // NOLINT
#include FT_FREETYPE_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

#include "logging.h"

#include "frame_data_analyser.hpp"
#include "gui_constants.hpp"
#include "platform_threading.hpp"
#include "shaders.hpp"
#include "src/gui/platform_gui.hpp"

#include "generated-src/fonts.h"

namespace {

// Constants
#define TEXT_COLOR_GREEN glm::vec3(0.5, 1.0F, 0.2F)
#define TEXT_COLOR_RED glm::vec3(1.0, 0.2F, 0.2F)

// Global variables
struct frame_data_point g_data_point = {};
float g_distance = 0;
player_state g_status = {};
bool g_game_hooked = false;

struct font_char {
    unsigned int texture_id; // ID handle of the glyph texture
    glm::ivec2 size; // Size of glyph
    glm::ivec2 bearing; // Offset from baseline to left/top of glyph
    unsigned int advance; // Horizontal offset to advance to next glyph
};

std::map<GLchar, font_char> font_chars;
unsigned int g_vao, g_vbo;
unsigned int g_shader_program;

class listener : public event_listener {
public:
    void frame_data(const frame_data_point frame_data) override;
    void distance(const float distance) override;
    void status(const player_state state) override;
    void game_hooked() override;
};

void listener::frame_data(const frame_data_point frame_data) {
    g_data_point = frame_data;
    log_info("startup frames: %d, frame advantage: %d", frame_data.startup_frames, frame_data.frame_advantage);
}

void listener::distance(const float distance) {
    g_distance = distance;
}

void listener::status(const player_state state) {
    g_status = state;
}

void listener::game_hooked() {
    g_game_hooked = true;
    platform_find_game_window();
}

listener g_listener;

void render_text(unsigned int shader, std::string text, float x, float y, float scale, glm::vec3 color) { // NOLINT
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(g_vao);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        const font_char ch = font_chars[*c];

        const float xpos = x + ((float) ch.bearing.x * scale);
        const float ypos = y - ((float) (ch.size.y - ch.bearing.y) * scale);

        const float w = (float) ch.size.x * scale;
        const float h = (float) ch.size.y * scale;

        // Update VBO for each character
        float vertices[6][4] = {{xpos, ypos + h, 0.0F, 0.0F},
                                {xpos, ypos, 0.0F, 1.0F},
                                {xpos + w, ypos, 1.0F, 1.0F},

                                {xpos, ypos + h, 0.0F, 0.0F},
                                {xpos + w, ypos, 1.0F, 1.0F},
                                {xpos + w, ypos + h, 1.0F, 0.0F}};

        // Render glyph to quad
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);

        // Update VBO
        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (float) (ch.advance >> (unsigned int) 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void render_line(std::string text, int line, glm::vec3 color) {
    render_text(g_shader_program, std::move(text), 0, (float) ((FONT_SIZE * line) + TEXT_MARGIN), 1, color);
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

bool setup_shaders() {
    g_shader_program = glCreateProgram();

    const unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    if (vs == 0) {
        log_error("failed to create vertex shader");
        return false;
    }
    glShaderSource(vs, 1, &VERTEX_SHADER_SOURCE, nullptr);
    glCompileShader(vs);

    const unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (fs == 0) {
        log_error("failed to create fragment shader");
        return false;
    }
    glShaderSource(fs, 1, &FRAGMENT_SHADER_SOURCE, nullptr);
    glCompileShader(fs);

    glAttachShader(g_shader_program, vs);
    glAttachShader(g_shader_program, fs);
    glLinkProgram(g_shader_program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    glm::mat4 projection = glm::ortho(0.0F, (float) MAX_WIDTH, 0.0F, (float) MAX_HEIGTH);
    glUseProgram(g_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(g_shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void set_blending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool load_font() {
    FT_Library ft = nullptr;
    if (FT_Init_FreeType(&ft) != 0) {
        return false;
    }

    FT_Face face = nullptr;
    if (FT_New_Memory_Face(ft, fonts_DejaVuSansMono_ttf, fonts_DejaVuSansMono_ttf_len, 0, &face) != 0) {
        log_error("font not found");
        return false;
    }

    // Glyph size
    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);

    // Disable byte-alignment restriction, as FreeType glyphs aren't aligned to 4-bytes
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) { // NOLINT
            continue;
        }

        // Generate texture
        unsigned int texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED, // Use GL_RED for single-channel grayscale image
                     (int) face->glyph->bitmap.width,
                     (int) face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        // Texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        const font_char character = {.texture_id = texture,
                                     .size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                                     .bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                                     .advance = static_cast<unsigned int>(face->glyph->advance.x)};

        font_chars.insert(std::pair<char, font_char>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Free FT resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return true;
}

bool setup_graphics() {
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        log_error("failed to load glad GL");
        return false;
    }

    if (!setup_shaders()) {
        log_error("setting up shaders has failed");
        return false;
    }

    set_blending();

    if (!load_font()) {
        log_error("loading font has failed");
        return false;
    }

    return true;
}

GLFWwindow *setup_gui() {
    GLFWwindow *window = create_window();
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

    if (!setup_graphics()) {
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
        if (frame_data_analyser::start(&g_listener)) {
            break;
        }

        if (frame_data_analyser::should_stop()) {
            log_debug("shutting down analyser");
            return;
        }

        log_debug("starting analyser again after timeout");
        gui_state_no_game();
        std::this_thread::sleep_for(std::chrono::seconds(ANALYSER_START_INTERVAL));
    }
}

void draw_game_state() {
    char buffer[50];

    (void) sprintf(buffer, DISTANCE, g_distance);
    render_line(buffer, 0, TEXT_COLOR_GREEN);

    (void) sprintf(buffer, STATUS, frame_data_analyser::player_status(g_status));
    render_line(buffer, 1, TEXT_COLOR_GREEN);

    (void) sprintf(buffer, FRAME_ADVANTAGE, g_data_point.frame_advantage);
    if (g_data_point.frame_advantage < 0) {
        render_line(buffer, 2, TEXT_COLOR_RED);
    } else {
        render_line(buffer, 2, TEXT_COLOR_GREEN);
    }

    (void) sprintf(buffer, STARTUP_FRAMES, g_data_point.startup_frames);
    render_line(buffer, 3, TEXT_COLOR_GREEN);
}

void draw_no_game() {
    render_line(NO_DISTANCE, 0, TEXT_COLOR_GREEN);
    render_line(NO_STATUS, 1, TEXT_COLOR_GREEN);
    render_line(NO_FRAME_ADVANTAGE, 2, TEXT_COLOR_GREEN);
    render_line(NO_STARTUP_FRAMES, 3, TEXT_COLOR_GREEN);
}

void gui_loop(GLFWwindow *window) {
    /* Loop until the user closes the window */
    while (glfwWindowShouldClose(window) == 0) {
        // Tick platform functions
        platform_update();

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        if (g_game_hooked) {
            draw_game_state();
        } else {
            draw_no_game();
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
}

void start_gui(GLFWwindow *window) {
    std::thread analyser_thread(&analyser_loop);
    set_realtime_prio(analyser_thread);

    gui_loop(window);

    // GUI has exited
    frame_data_analyser::stop();
    analyser_thread.join();
}

} // namespace

int main() {
    log_set_level(LOG_TRACE);

    GLFWwindow *window = setup_gui();
    if (window == nullptr) {
        return -1;
    }

    start_gui(window);

    return 0;
}
