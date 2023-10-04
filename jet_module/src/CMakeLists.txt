set(LIB_NAME jet_module)
set(MODULE_HEADERS_DIR ../include/jet_module)

set(SRC_Include common.h
                module_dll.h
                jet_module_impl.h
)

set(SRC_Srcs module_dll.cpp
             jet_module_impl.cpp
)

prepend_include(jet_module SRC_Include)
get_filename_component(CURRENT_FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
get_filename_component(CURRENT_FOLDER_NAME ${CURRENT_FOLDER_NAME} NAME)

source_group("module" FILES ${MODULE_HEADERS_DIR}/jet_module_impl.h
                            ${MODULE_HEADERS_DIR}/module_dll.h
                            module_dll.cpp
                            jet_module_impl.cpp
)


add_library(${LIB_NAME} SHARED ${SRC_Include}
                               ${SRC_Srcs}
)
add_library(${SDK_TARGET_NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_link_libraries(${LIB_NAME} PUBLIC  daq::opendaq)

target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
                                              $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
                                              $<INSTALL_INTERFACE:include>
)

# ${CMAKE_SHARED_LIBRARY_SUFFIX} returns an empty string. This is a temporary fix.
set(OPENDAQ_MODULE_SUFFIX ".module${CMAKE_SHARED_LIBRARY_SUFFIX}")
opendaq_set_module_properties(${LIB_NAME} ${PROJECT_VERSION_MAJOR})
# create_version_header(${LIB_NAME})

target_link_libraries(${LIB_NAME} PUBLIC ${ASAN_LINK_FLAGS} ${CMAKE_THREAD_LIBS_INIT} daq jetpeer jetpeerasync jsoncpp_lib pugixml)
