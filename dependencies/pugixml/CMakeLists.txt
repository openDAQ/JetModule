cmake_minimum_required(VERSION 3.24)

include(FetchContent)

set(pugixml_REQUIREDVERSION "1.10")

FetchContent_Declare(pugixml
    URL                         https://github.com/zeux/pugixml/archive/v${pugixml_REQUIREDVERSION}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP  FALSE
    FIND_PACKAGE_ARGS           ${pugixml_REQUIREDVERSION}
)

FetchContent_MakeAvailable(pugixml)
