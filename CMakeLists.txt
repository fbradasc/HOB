cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL TRUE)

project(Message VERSION 0.1.0)

# set(DUMP_ALL DUMP_ALL)
# set(DEBUG_WRITE DEBUG_WRITE)
# set(TEST_VARIANT_HOBS TEST_VARIANT_HOBS)
set(ENABLE_DYNAMIC_FIELDS ENABLE_DYNAMIC_FIELDS)
set(TEST_FIXED_HOBS TEST_FIXED_HOBS)

set(BASIC_TEST_SRC
    tests/basic/test.cpp)

set(SOCKET_SERVER_SRC
    tests/sockets/server.cpp)

set(SOCKET_CLIENT_SRC
    tests/sockets/client.cpp)

set(CMAKE_CXX_STANDARD 98)
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

add_executable(test_hob
               ${BASIC_TEST_SRC})

add_executable(test_server
               ${SOCKET_SERVER_SRC})

add_executable(test_client
               ${SOCKET_CLIENT_SRC})

if (NOT ANDROID)
    target_link_libraries(test_server pthread)

    target_link_libraries(test_client pthread)
endif()

target_compile_definitions(test_hob
                           PUBLIC
                           ${BINARY_ONLY}
                           ${TEST_VARIANT_HOBS}
                           ${TEST_FIXED_HOBS}
                           ${ENABLE_DYNAMIC_FIELDS}
                           ${DEBUG_WRITE})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
