cmake_minimum_required(VERSION 3.24)

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
