set(LIB_NAME jetModule)
set(MODULE_HEADERS_DIR "../include")

set(SRC_Include ${MODULE_HEADERS_DIR}/print_hello.h)
set(SRC_Srcs print_hello.cpp)

add_library(${LIB_NAME} SHARED ${SRC_Include} ${SRC_Srcs})
target_include_directories(${LIB_NAME} PUBLIC ${MODULE_HEADERS_DIR})