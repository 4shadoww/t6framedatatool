enable_testing()

add_executable(
  test_input_key
  test_input_key.cpp
)

target_link_libraries(
  test_input_key
  GTest::gtest_main
)

target_include_directories(test_input_key PRIVATE ${COMMON_SRC})
target_include_directories(test_input_key SYSTEM PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

gtest_discover_tests(test_input_key)
