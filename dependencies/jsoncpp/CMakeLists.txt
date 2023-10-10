cmake_minimum_required(VERSION 3.24)

include(FetchContent)

set(jsoncpp_REQUIREDVERSION "1.9.3")

set(BUILD_SHARED_LIBS                   ON)
set(JSONCPP_WITH_POST_BUILD_UNITTEST    OFF)
set(JSONCPP_WITH_TESTS                  OFF)

FetchContent_Declare(jsoncpp
    URL                         https://github.com/open-source-parsers/jsoncpp/archive/${jsoncpp_REQUIREDVERSION}.zip
    DOWNLOAD_EXTRACT_TIMESTAMP  FALSE
    FIND_PACKAGE_ARGS           ${jsoncpp_REQUIREDVERSION}
    SOURCE_DIR			        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

FetchContent_MakeAvailable(jsoncpp)
