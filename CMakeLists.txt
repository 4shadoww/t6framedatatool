cmake_minimum_required(VERSION 3.14)
project(Tekken6FrameDataTool)

option(DEBUG "Debug build" OFF)
option(UNIT_TESTING "Unit testing" OFF) # Build tests
option(GUI "Build GUI" OFF)

# Project level properties
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -pedantic -Wno-error=unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -pedantic")

# Lib types
set(UTILS_LIB_TYPE OBJECT)
set(MEMORY_READER_LIB_TYPE OBJECT)
set(COMMON_LIB_TYPE OBJECT)

if(DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -s")
endif()

if(UNIT_TESTING)
    enable_testing()
    add_subdirectory(tests)
else()
    add_subdirectory(src)
endif()
