set(TARGET t6framedata-gui)

set(SRCS opengl_gui.cpp platform_gui_linux.cpp)
set(LIBS glfw GL glad_gl_core_33 glm::glm freetype X11)

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

target_include_directories(${TARGET}
   INTERFACE ${GENERATED_SRCS}
)
