cmake_minimum_required(VERSION 3.24)

enable_testing()

set(JET_SERVER_TEST_NAME JetServerTest)
set(PROPERTY_CONVERTER_TEST_NAME PropertyConverterTest)
set(JET_SERVER_TEST_SOURCE jet_server_test.cpp)
set(PROPERTY_CONVERTER_TEST_SOURCE property_converter_test.cpp)

add_executable(${JET_SERVER_TEST_NAME} ${JET_SERVER_TEST_SOURCE})
target_link_libraries(${JET_SERVER_TEST_NAME} PUBLIC GTest::gtest_main JetModule daq::opendaq hbk jetpeer jetpeerasync jsoncpp_lib pugixml)
add_compile_definitions(MODULE_PATH="${CMAKE_BINARY_DIR}")

add_executable(${PROPERTY_CONVERTER_TEST_NAME} ${PROPERTY_CONVERTER_TEST_SOURCE})
target_link_libraries(${PROPERTY_CONVERTER_TEST_NAME} PUBLIC GTest::gtest_main JetModule daq::opendaq hbk jetpeer jetpeerasync jsoncpp_lib pugixml)
add_compile_definitions(MODULE_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)
gtest_discover_tests(${JET_SERVER_TEST_NAME} ${PROPERTY_CONVERTER_TEST_NAME}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
