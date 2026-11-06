enable_testing()

add_executable(test_refl_struct test_refl_struct.cpp)

target_link_libraries(
  test_refl_struct
  PRIVATE GTest::gtest_main
  PRIVATE memoryreader
  PRIVATE utils
)

target_include_directories(test_refl_struct SYSTEM PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

gtest_discover_tests(test_refl_struct)
