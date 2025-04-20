find_package(GTest REQUIRED PATHS 3rdparty)
include(GoogleTest)

set(SRC ${PROJECT_SOURCE_DIR}/src)
set(COMMON_SRC ${SRC}/common)
set(MEMORY_READER_SRC ${SRC}/memory_reader)
set(UTILS_SRC ${SRC}/utils)

add_subdirectory(ringbuffer)
