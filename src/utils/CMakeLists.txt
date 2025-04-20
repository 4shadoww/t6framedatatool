set(TARGET utils)
set(SRCS logging.c number_conversions.c)

add_library(${TARGET} ${UTILS_LIB_TYPE} ${SRCS})
target_include_directories(${TARGET} PUBLIC .)
