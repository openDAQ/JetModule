cmake_minimum_required(VERSION 3.24)

project(JetModule 
    VERSION 1.0.0 
    DESCRIPTION "Library for using JetPeer alongside with openDAQ"
    LANGUAGES C CXX)

##### Variables
set(HBKWORLD_REPO_PREFIX "git@github.com:hbkworld" CACHE STRING "Set this if using a repository mirror")
set(OPENDAQ_REPO_PREFIX "git@github.com:openDAQ" CACHE STRING "Set this if using a repository mirror")


##### Options
option(IGNORE_INSTALLED_SDK "Build SDK internally even if found on system"                           OFF)
option(COMPILE_REFERENCE_APPLICATION "Reference application is an example on how to use jetModule "  ON)

##### Dependencies
add_subdirectory(dependencies/openDAQ)
add_subdirectory(dependencies/jsoncpp)
add_subdirectory(dependencies/libhbk)
add_subdirectory(dependencies/pugixml)
add_subdirectory(dependencies/jetpeer-cpp)

add_subdirectory(src)
if(COMPILE_REFERENCE_APPLICATION)
    add_subdirectory(app)
endif()