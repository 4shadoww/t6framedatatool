set(TARGET common)
set(SRCS frame_data_analyser.cpp)

add_library(${TARGET} ${COMMON_LIB_TYPE} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
)
target_include_directories(${TARGET} PUBLIC .)
