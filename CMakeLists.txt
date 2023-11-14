cmake_minimum_required(VERSION 3.24)

##### TOOLCHAIN_PATH has to be set from command by a developer
set(TOOLCHAIN_PATH "$ENV{TOOLCHAIN_PATH}")
message(STATUS "$Toolchain path: {TOOLCHAIN_PATH}")
##### Finding toolchain file
if(TOOLCHAIN_PATH)
    find_file(CMAKE_TOOLCHAIN_FILE NAMES "toolchainfile.cmake" PATHS "${TOOLCHAIN_PATH}" PATH_SUFFIXES "share/buildroot")
    if(NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
        message(FATAL_ERROR
            "Can't find toolchainfile.cmake. "
            "Did you forget to assign value to TOOLCHAIN_PATH environmental variable (see README)?")
    else()
        message(STATUS "Toolchain file location: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

project(JetModule 
    VERSION 1.0.0 
    DESCRIPTION "Library for using JetPeer alongside with openDAQ"
    LANGUAGES C CXX)

##### Variables
set(OPENDAQ_REPO_PREFIX "git@github.com:blueberrydaq" CACHE STRING "Set this if using a repository mirror")
set(OPEN_SOURCE_OPENDAQ_REPO_PREFIX "git@github.com:openDAQ" CACHE STRING "Set this if using a repository mirror")


##### Options
option(IGNORE_INSTALLED_SDK "Build SDK internally even if found on system"                           OFF)
option(COMPILE_REFERENCE_APPLICATION "Reference application is an example on how to use jetModule "  ON)

##### Dependencies
add_subdirectory(dependencies/openDAQ)
add_subdirectory(dependencies/jsoncpp)
add_subdirectory(dependencies/libopenDAQ)
add_subdirectory(dependencies/pugixml)
add_subdirectory(dependencies/jetPeerCpp)

add_subdirectory(src)
if(COMPILE_REFERENCE_APPLICATION)
    add_subdirectory(app)
endif()