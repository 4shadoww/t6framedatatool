set(TARGET common)

set(SRCS frame_data_analyser.cpp)

if(WIN32)
    #set(SRCS )
elseif(UNIX)
    set(SRCS ${SRCS} platform_threading_linux.cpp)
endif()

add_library(${TARGET} ${COMMON_LIB_TYPE} ${SRCS})
target_link_libraries(${TARGET}
    PRIVATE utils
    PRIVATE memoryreader
)
target_include_directories(${TARGET} PUBLIC .)
