set(TARGET dispatcher)

add_library(${TARGET} INTERFACE)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
