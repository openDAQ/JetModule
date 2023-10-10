cmake_minimum_required(VERSION 3.24)

project(jetModule 
    VERSION 1.0.0 
    DESCRIPTION "Library for using JetPeer alongside with openDAQ"
    LANGUAGES C CXX)

##### Variables
# set(OPENDAQ_REPO_PREFIX "git@github.com:blueberrydaq" CACHE STRING "Set this if using a repository mirror")


##### Options
option(IGNORE_INSTALLED_SDK "Build SDK internally even if found on system"                       ON)
option(COMPILE_REFERENCE_APPLICATION "Reference application is an example on how to use jetModule "  OFF)

##### Dependencies
add_subdirectory(dependencies/openDAQ)
add_subdirectory(dependencies/jsoncpp)
add_subdirectory(dependencies/libopenDAQ)
add_subdirectory(dependencies/pugixml)
add_subdirectory(dependencies/jetPeerCpp)

add_subdirectory(src)
add_subdirectory(app)


# install(TARGETS ${PROJECT_NAME})