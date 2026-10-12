set(TARGET memoryreader)

set(COMMON_SRCS game_state_reader.c moveset_structs.c)

if(WIN32)
    set(SRCS ${COMMON_SRCS} memory_reader_windows.c)
elseif(UNIX)
    set(SRCS ${COMMON_SRCS} memory_reader_linux.c)
else()
    message(FATAL_ERROR this platfrom is not supported)
endif()

add_library(${TARGET} ${MEMORY_READER_LIB_TYPE} ${SRCS})
set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET}
    PUBLIC utils
    PUBLIC refl
)
# refl is an object library, so its objects do not follow this link into executables.
target_sources(${TARGET} INTERFACE $<TARGET_OBJECTS:refl>)
