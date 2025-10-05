add_subdirectory(${3RDPARTY_LIBS}/googletest googletest)
include(GoogleTest)

set(COMMON_SRC ${SRCS}/common)
set(MEMORY_READER_SRC ${SRCS}/memory_reader)
set(UTILS_SRC ${SRCS}/utils)

add_subdirectory(ringbuffer)
add_subdirectory(print_framedata)
