cmake_minimum_required(VERSION 3.24)

include(FetchContent)

set(hbk_REQUIREDVERSION "2.0.0")

set(BUILD_SHARED_LIBS        ON)
set(HBK_GENERATE_DOC        OFF)
set(HBK_POST_BUILD_UNITTEST OFF)
set(HBK_TOOLS               OFF)
set(HBK_HARDWARE             ON)

find_package(hbk ${hbk_REQUIREDVERSION} QUIET)

if(NOT hbk_FOUND)

    FetchContent_Declare(hbk
        GIT_REPOSITORY          "${HBKWORLD_REPO_PREFIX}/libhbk.git"
        GIT_TAG				    v${hbk_REQUIREDVERSION}
        GIT_SHALLOW             TRUE
        SOURCE_DIR			    ${CMAKE_CURRENT_SOURCE_DIR}/src
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_MakeAvailable(hbk)

endif()
