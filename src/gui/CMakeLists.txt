set(TARGET t6framedata-gui)

if(X11)
    set(SRCS x11_gui.cpp)
    set(LIBS X11 Xext Xfixes)
else()
    set(SRCS opengl_gui.cpp gui_functions_linux.cpp)
    set(LIBS glfw GL glad_gl_core_33 glm::glm freetype X11)
endif()

if(DEBUG)
    set(TARGET ${TARGET}-debug)
endif()

add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
    PRIVATE ${LIBS}
)
