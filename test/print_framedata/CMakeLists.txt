set(TARGET print_framedata)
set(SRCS print_framedata.cpp)

set(TARGET ${TARGET})

add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
)
