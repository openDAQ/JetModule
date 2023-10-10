set(LIB_NAME jetModule)
set(MODULE_HEADERS_DIR "../include")

set(SRC_Include 
    ${MODULE_HEADERS_DIR}/common.h
    ${MODULE_HEADERS_DIR}/version.h
    ${MODULE_HEADERS_DIR}/module_dll.h
    ${MODULE_HEADERS_DIR}/jet_module_impl.h
    ${MODULE_HEADERS_DIR}/jet.h)

set(SRC_Srcs 
    module_dll.cpp
    jet_module_impl.cpp
    jet.cpp)

add_library(${LIB_NAME} SHARED ${SRC_Include} ${SRC_Srcs})
target_include_directories(${LIB_NAME} PUBLIC ${MODULE_HEADERS_DIR})
target_link_libraries(${LIB_NAME} PUBLIC daq::opendaq)