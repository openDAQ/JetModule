set(EXECUTABLE_NAME ref_app)

# Reference application is only compiled if the option is enabled
if(COMPILE_REFERENCE_APPLICATION)
    add_executable(${EXECUTABLE_NAME} ref_app.cpp)
    target_link_libraries(${EXECUTABLE_NAME} jetModule)
endif()