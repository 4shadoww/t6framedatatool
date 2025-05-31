# Defines source directories based on the target type

# Library targets
add_subdirectory(utils) # C code
add_subdirectory(memory_reader) # C code
add_subdirectory(common) # C++ code

if(GUI)
    add_subdirectory(gui)
endif()
add_subdirectory(cli)
