cmake_minimum_required(VERSION 3.10)

set(TARGET memoryreader)
# TODO: multi-target
set(SRCS memory_reader_linux.c)

add_library(${TARGET} ${MEMORY_READER_LIB_TYPE} ${SRCS})
set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET}
    PUBLIC utils
)
