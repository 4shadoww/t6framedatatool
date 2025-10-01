cmake_minimum_required(VERSION 3.14)
project(Tekken6FrameDataTool)

option(DEBUG "Debug build" OFF)
option(UNIT_TESTING "Unit testing" OFF) # Build tests
option(GUI "Build GUI" OFF)

# Project level properties
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Lib types
set(UTILS_LIB_TYPE OBJECT)
set(MEMORY_READER_LIB_TYPE OBJECT)
set(COMMON_LIB_TYPE OBJECT)

# Set global variables
set(3RDPARTY_LIBS ${PROJECT_SOURCE_DIR}/3rdparty)
set(SRCS ${PROJECT_SOURCE_DIR}/src)

# 3rdparty library imports
if(GUI)
    # Glfw
    add_subdirectory(${3RDPARTY_LIBS}/glfw glfw)

    # GLAD
    add_subdirectory("${3RDPARTY_LIBS}/glad/cmake" glad_cmake)
    glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

    # FreeType
    add_subdirectory("${3RDPARTY_LIBS}/freetype" freetype)

    # Glm
    add_subdirectory("${3RDPARTY_LIBS}/glm" glm)
endif()

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
