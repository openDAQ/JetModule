cmake_minimum_required(VERSION 3.24)

include(FetchContent)

set(jetpeer-cpp_REQUIREDVERSION "3.0.0")

set(BUILD_SHARED_LIBS ON)

include(FindJetPeer.cmake)
FindJetPeer("${jetpeer-cpp_REQUIREDVERSION}")
if(NOT JetPeer_FOUND)

    FetchContent_Declare(jetpeer-cpp
        GIT_REPOSITORY  "${HBKWORLD_REPO_PREFIX}/jetpeer-cpp.git"
	    GIT_TAG			v${jetpeer-cpp_REQUIREDVERSION}
        GIT_SHALLOW     TRUE
        SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/src
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_MakeAvailable(jetpeer-cpp)

endif()
