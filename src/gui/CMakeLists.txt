set(TARGET t6framedata-gui)

set(SRCS opengl_gui.cpp)
set(LIBS glfw glad_gl_core_33 glm::glm freetype)

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
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
    PRIVATE ${LIBS}
)

if(WIN32)
    # For some reason Windows needs this
    target_include_directories(${TARGET} PRIVATE ${3RDPARTY_LIBS})
endif()

target_include_directories(${TARGET}
   INTERFACE ${GENERATED_SRCS}
)
