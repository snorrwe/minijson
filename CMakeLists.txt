cmake_minimum_required(VERSION 3.0)
project(MiniJson VERSION 0.1.0 LANGUAGES CXX)
option(test "Build tests." OFF)
option(ci "Enable additional error flags." OFF)

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

    enable_testing()

    # Download and unpack googletest at configure time
    configure_file(GtestCmakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

    # The gtest/gtest_main targets carry header search path
    # dependencies automatically when using CMake 2.8.11 or
    # later. Otherwise we have to add them here ourselves.
    if (CMAKE_VERSION VERSION_LESS 2.8.11)
        include_directories("${gtest_SOURCE_DIR}/include")
    endif()


    add_executable(tests "${PROJECT_SOURCE_DIR}/test/test_json_parser.cpp" "${PROJECT_SOURCE_DIR}/test/test_json_serializer.cpp")
    target_link_libraries(tests mini_json gtest_main ${CMAKE_THREAD_LIBS_INIT})
    set_property(TARGET tests PROPERTY CXX_STANDARD 17)
    set_property(TARGET tests PROPERTY CXX_STANDARD_REQUIRED ON)

    add_test(NAME all_json_tests COMMAND tests)

endif()
