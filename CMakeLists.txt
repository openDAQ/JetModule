cmake_minimum_required(VERSION 3.24)

project(JetModule
    DESCRIPTION "Library for publishing openDAQ device structure as Jet states"
    LANGUAGES C CXX)

##### Variables
set(HBKWORLD_REPO_PREFIX "https://github.com/hbkworld" CACHE STRING "Set this if using a repository mirror")
set(OPENDAQ_REPO_PREFIX "https://github.com/openDAQ" CACHE STRING "Set this if using a repository mirror")

##### Options
option(JET_MODULE_ENABLE_TESTS "Enable testing"                                                                OFF)
option(COMPILE_REFERENCE_APPLICATION "Compiles reference application which can be used for testing JetModule"  OFF)
option(IGNORE_INSTALLED_SDK "Build SDK internally even if found on system"                                     OFF)

##### Dependencies
add_subdirectory(dependencies/openDAQ)
add_subdirectory(dependencies/jsoncpp)
add_subdirectory(dependencies/libhbk)
add_subdirectory(dependencies/pugixml)
add_subdirectory(dependencies/jetpeer-cpp)
add_subdirectory(dependencies/gtest)

add_subdirectory(src)
if (JET_MODULE_ENABLE_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()
if (COMPILE_REFERENCE_APPLICATION)
    add_subdirectory(app)
endif()