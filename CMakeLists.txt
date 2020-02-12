cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL TRUE)

project(Message VERSION 0.1.0)

set(BASIC_TEST_SRC
    tests/basic/test.cpp)

set(SOCKET_SERVER_SRC
    tests/sockets/fdstreambuf.cpp
    tests/sockets/server.cpp)

set(SOCKET_CLIENT_SRC
    tests/sockets/fdstreambuf.cpp
    tests/sockets/client.cpp)

# set(CMAKE_CXX_STANDARD 98)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_compile_options(-Wall 
                    -Wextra 
                    -Wcast-align 
                    -Wcast-qual 
                    -Wctor-dtor-privacy 
                    -Wdisabled-optimization 
                    -Winit-self 
                    -Wmissing-include-dirs 
                    -Wold-style-cast 
                    -Woverloaded-virtual 
                    -Wredundant-decls 
                    -Wshadow 
                    -Wsign-promo 
                    -Wstrict-null-sentinel 
                    -Wswitch-default 
                    -Wundef 
                    -Werror 
                    -Wno-unused)

add_executable(test_message_on_file
               ${BASIC_TEST_SRC})

add_executable(test_message_on_iostream
               ${BASIC_TEST_SRC})

add_executable(test_message_on_separate_stringstream
               ${BASIC_TEST_SRC})

add_executable(test_server
               ${SOCKET_SERVER_SRC})

add_executable(test_client
               ${SOCKET_CLIENT_SRC})

target_compile_definitions(test_message_on_iostream
                           PUBLIC
                           ${BINARY_ONLY})

target_compile_definitions(test_message_on_file
                           PUBLIC
                           OUTPUT_ON_FILE ${BINARY_ONLY})

target_compile_definitions(test_message_on_separate_stringstream
                           PUBLIC
                           SEPARATE_IN_AND_OUT_STRING_STREAMS ${BINARY_ONLY})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
