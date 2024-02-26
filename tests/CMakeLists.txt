cmake_minimum_required(VERSION 3.24)

enable_testing()

set(JET_SERVER_TEST_NAME JetServerTest)
set(CONVERTERS_TEST_NAME ConvertersTest)
set(JET_SERVER_TEST_SOURCE jet_server_test.cpp)
set(CONVERTERS_TEST_SOURCE converters_test.cpp)

add_executable(${JET_SERVER_TEST_NAME} ${JET_SERVER_TEST_SOURCE})
target_link_libraries(${JET_SERVER_TEST_NAME} PUBLIC GTest::gtest_main JetModule daq::opendaq hbk jetpeer jetpeerasync jsoncpp_lib pugixml)
add_compile_definitions(MODULE_PATH="${CMAKE_BINARY_DIR}")

add_executable(${CONVERTERS_TEST_NAME} ${CONVERTERS_TEST_SOURCE})
target_link_libraries(${CONVERTERS_TEST_NAME} PUBLIC GTest::gtest_main JetModule daq::opendaq hbk jetpeer jetpeerasync jsoncpp_lib pugixml)
add_compile_definitions(MODULE_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)
gtest_discover_tests(${JET_SERVER_TEST_NAME} ${CONVERTERS_TEST_NAME}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
