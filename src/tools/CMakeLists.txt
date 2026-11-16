set(TARGET t6framedump)
set(SRCS frame_dump_tool.cpp)

if(DEBUG)
    set(TARGET ${TARGET}-debug)
endif()

add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
)

set(INPUT_TARGET t6inputdump)
set(INPUT_SRCS input_dump_tool.cpp)

if(DEBUG)
    set(INPUT_TARGET ${INPUT_TARGET}-debug)
endif()

add_executable(${INPUT_TARGET} ${INPUT_SRCS})
target_link_libraries(${INPUT_TARGET}
    PRIVATE utils
    PRIVATE memoryreader
    PRIVATE common
)
