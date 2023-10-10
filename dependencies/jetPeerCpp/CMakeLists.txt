cmake_minimum_required(VERSION 3.24)

include(FetchContent)

set(jetPeerCpp_REQUIREDVERSION "2.0.9")

find_package(jetpeerasync   ${jetPeerCpp_REQUIREDVERSION} QUIET)
find_package(jetpeer        ${jetPeerCpp_REQUIREDVERSION} QUIET)

if(NOT jetpeer_FOUND OR NOT jetpeerasync_FOUND)

    FetchContent_Declare(jetPeerCpp
        GIT_REPOSITORY  "${OPENDAQ_REPO_PREFIX}/jetPeerCpp.git"
	    GIT_TAG			v${jetPeerCpp_REQUIREDVERSION}
        GIT_SHALLOW     TRUE
        SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    FetchContent_MakeAvailable(jetPeerCpp)

endif()
