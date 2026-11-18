set(TARGET configuration)
set(SRCS configuration_manager.cpp)

add_library(${TARGET} ${CONFIGURATION_LIB_TYPE} ${SRCS})
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET} PUBLIC tomlplusplus::tomlplusplus)
