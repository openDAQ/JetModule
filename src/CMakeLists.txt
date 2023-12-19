set(LIB_NAME ${PROJECT_NAME})
set(MODULE_HEADERS_DIR "${PROJECT_SOURCE_DIR}/include")

# Header files
set(SRC_Include 
    common.h
    module_dll.h
    jet_module_impl.h
    json_daq_conversion.h
    jet_server.h
    version.h
)

# Source files
set(SRC_Srcs 
    module_dll.cpp
    jet_module_impl.cpp
    json_daq_conversion.cpp
    jet_server.cpp
)

# Prepend path to header files
list(TRANSFORM SRC_Include PREPEND "${MODULE_HEADERS_DIR}/")

source_group("module" FILES 
    ${MODULE_HEADERS_DIR}/module_dll.h
    ${MODULE_HEADERS_DIR}/jet_module_impl.h
    ${MODULE_HEADERS_DIR}/jet.h
    module_dll.cpp
    jet_module_impl.cpp
    jet.cpp
)

# Library creation
add_library(${LIB_NAME} SHARED 
    ${SRC_Include} 
    ${SRC_Srcs}
)
add_library(${SDK_TARGET_NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})
target_include_directories(${LIB_NAME} PUBLIC ${MODULE_HEADERS_DIR})
target_link_libraries(${LIB_NAME} PUBLIC daq::opendaq hbk jetpeer jetpeerasync jsoncpp_lib pugixml)

target_compile_definitions(${LIB_NAME} PUBLIC OPENDAQ_THREAD_SAFE)
target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${LIB_NAME}>
                                              $<INSTALL_INTERFACE:${LIB_NAME}>
)

set(OPENDAQ_MODULE_SUFFIX ".module${CMAKE_SHARED_LIBRARY_SUFFIX}")
opendaq_set_module_properties(${LIB_NAME} ${PROJECT_VERSION_MAJOR})

install(TARGETS ${LIB_NAME} 
        EXPORT ${LIB_NAME}
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
file(GLOB MODULE_HEADERS "${MODULE_HEADERS_DIR}/*.h")
install(FILES ${MODULE_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME}")