find_package(GTest REQUIRED PATHS 3rdparty)
include(GoogleTest)

set(COMMON_SRC ${SRCS}/common)
set(MEMORY_READER_SRC ${SRCS}/memory_reader)
set(UTILS_SRC ${SRCS}/utils)

add_subdirectory(ringbuffer)
