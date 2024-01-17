cmake_minimum_required(VERSION 3.24)

set(TEST_NAME JetModuleTests)

enable_testing()

set(TEST_SOURCES
    jet_server_test.cpp
)

add_executable(${TEST_NAME} ${TEST_SOURCES})
target_link_libraries(${TEST_NAME} PUBLIC GTest::gtest_main JetModule daq::opendaq hbk jetpeer jetpeerasync jsoncpp_lib pugixml)
add_compile_definitions(MODULE_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)
gtest_discover_tests(${TEST_NAME}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
