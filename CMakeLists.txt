cmake_minimum_required(VERSION 3.10)
project(Tekken6FrameDataTool)

# Project level properties
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Lib types
set(UTILS_LIB_TYPE OBJECT)
set(MEMORY_READER_LIB_TYPE OBJECT)
set(COMMON_LIB_TYPE OBJECT)

add_subdirectory(src)
