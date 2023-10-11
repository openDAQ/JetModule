set(EXECUTABLE_NAME ref_app)

add_executable(${EXECUTABLE_NAME} ref_app.cpp)
target_link_libraries(${EXECUTABLE_NAME} jetModule)
