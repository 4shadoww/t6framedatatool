set(TARGET memoryreader)

if(WIN32)
    set(SRCS game_state_reader.c memory_reader_windows.c)
elseif(UNIX)
    set(SRCS game_state_reader.c memory_reader_linux.c)
else()
    message(FATAL_ERROR this platfrom is not supported)
endif()

add_library(${TARGET} ${MEMORY_READER_LIB_TYPE} ${SRCS})
set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET}
    PUBLIC utils
)
