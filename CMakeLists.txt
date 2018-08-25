cmake_minimum_required(VERSION 3.0)
project(mini_json VERSION 0.1.0 LANGUAGES CXX)
option(test "Build tests." ON)
option(ci "Enable additional error flags." ON)

#[ library ]

add_library(mini_json INTERFACE)
target_include_directories(mini_json INTERFACE src/)

if(ci)

    if(MSVC)
        target_compile_options(mini_json INTERFACE /W4 /WX)
    else()
        target_compile_options(mini_json INTERFACE -Wall -Wextra -pedantic -Werror)
    endif()

endif()

#[ tests ]

if(test)

    find_package (Threads)

    set(GTEST_ROOT_DIR  "${PROJECT_SOURCE_DIR}/lib/googletest")
    include_directories("${GTEST_ROOT_DIR}/googletest" "${GTEST_ROOT_DIR}/googletest/include")
    add_library(gtest STATIC "${GTEST_ROOT_DIR}/googletest/src/gtest-all.cc" "${GTEST_ROOT_DIR}/googletest/src/gtest_main.cc")
    set_property(TARGET gtest PROPERTY CXX_STANDARD 11)
    set_property(TARGET gtest PROPERTY CXX_STANDARD_REQUIRED ON)

    add_executable(tests "${PROJECT_SOURCE_DIR}/test/main.cpp" "${PROJECT_SOURCE_DIR}/test/test_json_parser.cpp" "${PROJECT_SOURCE_DIR}/test/test_json_serializer.cpp")
    target_link_libraries(tests gtest mini_json ${CMAKE_THREAD_LIBS_INIT})
    set_property(TARGET tests PROPERTY CXX_STANDARD 17)
    set_property(TARGET tests PROPERTY CXX_STANDARD_REQUIRED ON)

endif()
