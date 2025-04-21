set(TARGET t6framedata-gui)
set(SRCS gui.cpp)

add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
    PRIVATE xosd
)
