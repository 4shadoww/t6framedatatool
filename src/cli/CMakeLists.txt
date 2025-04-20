set(TARGET t6framedata)
set(SRCS cli.cpp)

add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
)
