set(TARGET refl)
set(SRCS refl_struct.c)

add_library(${TARGET} ${REFL_LIB_TYPE} ${SRCS})
set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET}
    PUBLIC utils
)
