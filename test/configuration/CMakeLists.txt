enable_testing()

add_executable(
  test_configuration_manager
  test_configuration_manager.cpp
)

target_link_libraries(
  test_configuration_manager
  PRIVATE GTest::gtest_main
  PRIVATE configuration
  PRIVATE utils
)

target_include_directories(test_configuration_manager SYSTEM PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

gtest_discover_tests(test_configuration_manager)
