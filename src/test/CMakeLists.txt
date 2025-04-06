cmake_minimum_required(VERSION 3.10)

set(TARGET print_frame_data)
set(SRCS print_frame_data.cpp)

add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
)
