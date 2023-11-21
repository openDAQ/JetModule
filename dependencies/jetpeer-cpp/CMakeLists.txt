cmake_minimum_required(VERSION 3.24)

include(FetchContent)

set(jetpeer-cpp_REQUIREDVERSION "3.0.0")

find_package(jetpeerasync   ${jetpeer-cpp_REQUIREDVERSION} QUIET)
find_package(jetpeer        ${jetpeer-cpp_REQUIREDVERSION} QUIET)

if(NOT jetpeer_FOUND OR NOT jetpeerasync_FOUND)

    FetchContent_Declare(jetpeer-cpp
        GIT_REPOSITORY  "${HBKWORLD_REPO_PREFIX}/jetpeer-cpp.git"
	    GIT_TAG			v${jetpeer-cpp_REQUIREDVERSION}
        GIT_SHALLOW     TRUE
        SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    FetchContent_MakeAvailable(jetpeer-cpp)

endif()
