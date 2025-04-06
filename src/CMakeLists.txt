cmake_minimum_required(VERSION 3.10)

# Defines source directories based on the target type

# Library targets
add_subdirectory(utils) # C code
add_subdirectory(memory_reader) # C code
add_subdirectory(common) # C++ code

# TODO: add flags to select if cli or gui is build (or just a library)
add_subdirectory(cli)

# TODO: control building tests with a flag
add_subdirectory(test)
