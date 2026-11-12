set(TARGET t6framedata-gui)

set(IMGUI_DIR ${3RDPARTY_LIBS}/imgui)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui SYSTEM PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC
    glfw
    glad_gl_core_33
)

set(SRCS opengl_gui.cpp
         analyser_event_router.cpp
         component/overlay_canvas.cpp
         view/command_history_view.cpp
         view/frame_data_view.cpp
         viewmodel/frame_data_view_model.cpp
)
set(LIBS glfw glad_gl_core_33 imgui)

if(WIN32)
    set(SRCS ${SRCS} platform_gui_windows.cpp)
    set(LIBS ${LIBS} user32)
elseif(UNIX)
    set(SRCS ${SRCS} platform_gui_linux.cpp)
    set(LIBS ${LIBS} X11 GL)
endif()

if(DEBUG)
    set(TARGET ${TARGET}-debug)
endif()

add_executable(${TARGET} ${SRCS})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
    PRIVATE ${LIBS}
)

if(WIN32)
    # For some reason Windows needs this
    target_include_directories(${TARGET} SYSTEM PRIVATE ${3RDPARTY_LIBS})
endif()
