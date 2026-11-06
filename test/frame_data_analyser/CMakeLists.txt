add_executable(
  test_frame_data_analyser
  test_frame_data_analyser.cpp
)

target_link_libraries(
  test_frame_data_analyser
  PRIVATE GTest::gtest_main
  PRIVATE common
  PRIVATE memoryreader
  PRIVATE utils
)

target_include_directories(test_frame_data_analyser SYSTEM PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_compile_definitions(test_frame_data_analyser PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../test_data")

gtest_discover_tests(test_frame_data_analyser)
