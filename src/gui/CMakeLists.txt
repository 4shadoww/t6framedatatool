set(TARGET t6framedata-gui)
set(SRCS x11_gui.cpp)

if(DEBUG)
    set(TARGET ${TARGET}-debug)
endif()

add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
    PRIVATE X11
    PRIVATE Xext
    PRIVATE Xfixes
)
