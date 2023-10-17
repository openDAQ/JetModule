set(EXECUTABLE_NAME ref_app)

add_executable(${EXECUTABLE_NAME} ref_app.cpp)
target_link_libraries(${EXECUTABLE_NAME} jetModule daq::opendaq)
add_compile_definitions(MODULE_PATH="${CMAKE_BINARY_DIR}")