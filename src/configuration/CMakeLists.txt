set(TARGET configuration)
set(SRCS configuration_manager.cpp)

if(WIN32)
    set(SRCS ${SRCS} platform_configuration_windows.cpp)
elseif(UNIX)
    set(SRCS ${SRCS} platform_configuration_linux.cpp)
endif()

add_library(${TARGET} ${CONFIGURATION_LIB_TYPE} ${SRCS})
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET}
    PUBLIC tomlplusplus::tomlplusplus
    PRIVATE utils
)
