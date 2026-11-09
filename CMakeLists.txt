cmake_minimum_required(VERSION 3.14)
project(Tekken6FrameDataTool)

option(DEBUG "Debug build" OFF)
option(UNIT_TESTING "Unit testing" OFF) # Build tests
option(GUI "Build GUI" OFF)
option(STATIC_RUNTIME "Link C++ runtime statically" ON)

# Project level properties
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Lib types
set(UTILS_LIB_TYPE OBJECT)
set(REFL_LIB_TYPE OBJECT)
set(MEMORY_READER_LIB_TYPE OBJECT)
set(COMMON_LIB_TYPE OBJECT)

# Set global variables
set(3RDPARTY_LIBS ${PROJECT_SOURCE_DIR}/3rdparty)
set(SRCS ${PROJECT_SOURCE_DIR}/src)

# Third-party headers are not project code. Mark their include directories as
# system headers so -Werror and clang-tidy report only src/ and test/.
function(mark_includes_as_system target)
    if(NOT TARGET ${target})
        return()
    endif()
    get_target_property(_incs ${target} INTERFACE_INCLUDE_DIRECTORIES)
    if(_incs)
        set_property(TARGET ${target} PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${_incs})
    endif()
endfunction()

# 3rdparty library imports
if(GUI)
    # Glfw
    add_subdirectory(${3RDPARTY_LIBS}/glfw glfw)

    # GLAD
    add_subdirectory("${3RDPARTY_LIBS}/glad/cmake" glad_cmake)
    glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

    foreach(tgt glfw glad_gl_core_33)
        mark_includes_as_system(${tgt})
    endforeach()
endif()

if(DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

if(STATIC_RUNTIME AND UNIX AND NOT APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_link_options(-static-libgcc -static-libstdc++)
endif()

add_subdirectory(src)

if(UNIT_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
