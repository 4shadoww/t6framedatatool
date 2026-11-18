# Defines source directories based on the target type
# Source tree flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -pedantic -Wno-error=unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -pedantic")

# Library targets
add_subdirectory(utils) # C code
add_subdirectory(refl) # C code
add_subdirectory(memory_reader) # C code
add_subdirectory(dispatcher) # C++ headers
add_subdirectory(common) # C++ code
add_subdirectory(configuration) # C++ headers

if(GUI)
    add_subdirectory(gui)
endif()
add_subdirectory(cli)
add_subdirectory(tools)
