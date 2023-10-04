cmake_minimum_required(VERSION 3.24)
# set_cmake_folder_context(TARGET_FOLDER_NAME)

project(jet-module VERSION 1.0.0)

option(IGNORE_INSTALLED_SDK "Build SDK internally even if found on system"          OFF)

set(OPENDAQ_REPO_PREFIX "git@github.com:blueberrydaq" CACHE STRING "Set this if using a repository mirror")

add_subdirectory(openDAQ-SDK)

include(lib/jsoncpp/CMakeLists.txt)
add_subdirectory(lib/libopenDAQ)
add_subdirectory(lib/pugixml)
add_subdirectory(lib/jetPeerCpp)

add_subdirectory(jet_module)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD            17
    CXX_STANDARD_REQUIRED   ON
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        daq::opendaq)

install(TARGETS ${PROJECT_NAME})