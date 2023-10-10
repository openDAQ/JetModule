cmake_minimum_required(VERSION 3.24)

include(FetchContent)

set(daq_REQUIREDVERSION "0.0.12")

set(DAQ_SYSTEM_POST_BUILD_UNITTEST OFF)

find_package(daq ${daq_REQUIREDVERSION} QUIET)

if(NOT daq_FOUND)

    FetchContent_Declare(daq
        GIT_REPOSITORY          "${OPENDAQ_REPO_PREFIX}/libopenDAQ.git"
        GIT_TAG				    v${daq_REQUIREDVERSION}
        GIT_SHALLOW             TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_MakeAvailable(daq)

endif()
