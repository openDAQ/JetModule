cmake_minimum_required(VERSION 3.24)

set(TEST_NAME JetModuleTests)

include(FetchContent)

set(gtest_REQUIREDVERSION "1.12.0")

find_package(GTest ${gtest_REQUIREDVERSION} )

if(GTest_FOUND)
  message(STATUS "Found GTest: ${GTest_VERSION} ${GTest_CONFIG}")
else()
  FetchContent_Declare(
    GTest
    URL https://github.com/google/googletest/archive/release-${gtest_REQUIREDVERSION}.zip
  )
  FetchContent_MakeAvailable(GTest)
endif()


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
