enable_testing()

add_executable(
  test_ring_buffer
  test_ring_buffer.cpp
)

target_link_libraries(
  test_ring_buffer
  GTest::gtest_main
)

include_directories(${COMMON_SRC}
                    ${gtest_SOURCE_DIR}/include
                    ${gtest_SOURCE_DIR})

gtest_discover_tests(test_ring_buffer)
