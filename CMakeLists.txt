cmake_minimum_required(VERSION 3.8)
project(MiniJson VERSION 0.1.0 LANGUAGES CXX)
option(test "Build tests." OFF)
option(ci "Enable additional error flags." OFF)

if(CMAKE_COPILER_ID_GNUCC)
    option(coverage "Enable coverage reporting for gcc/clang" OFF)
endif()

#[ library ]

add_library(mini_json INTERFACE)
target_include_directories(mini_json INTERFACE src/)

if(ci)

    if(MSVC)
        target_compile_options(mini_json INTERFACE /W4 /WX)
    else()
        target_compile_options(mini_json INTERFACE -Wall -Wextra -pedantic -Werror)

        if(coverage)
            target_compile_options(mini_json INTERFACE --coverage -O0)
        endif()

    endif()

endif()

#[ tests ]

if(test)

    enable_testing()

    # Download and unpack googletest at configure time
    configure_file(GtestCMakeLists.txt.in googletest-download/CMakeLists.txt)
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
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src ${CMAKE_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)

    add_executable(tests "${PROJECT_SOURCE_DIR}/test/test_json_parser.cpp" "${PROJECT_SOURCE_DIR}/test/test_json_serializer.cpp")
    target_link_libraries(tests mini_json gtest_main ${CMAKE_THREAD_LIBS_INIT})
    set_property(TARGET tests PROPERTY CXX_STANDARD 17)
    set_property(TARGET tests PROPERTY CXX_STANDARD_REQUIRED ON)

    add_test(NAME all_json_tests COMMAND tests)

endif()
