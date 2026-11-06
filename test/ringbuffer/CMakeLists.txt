enable_testing()

add_executable(
  test_ring_buffer
  test_ring_buffer.cpp
)

target_link_libraries(
  test_ring_buffer
  GTest::gtest_main
)

target_include_directories(test_ring_buffer PRIVATE ${COMMON_SRC})
target_include_directories(test_ring_buffer SYSTEM PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

gtest_discover_tests(test_ring_buffer)
