cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(ouroboros)

option(CMAKE_EXPORT_COMPILE_COMMANDS                            ON)
option(OUROBOROS_USE_CLANG "use clang"                          OFF)
option(OUROBOROS_EXAMPLES_ENABLED "the examples are enabled"    ON)
option(OUROBOROS_TOOLS_ENABLED "the tools are enabled"          ON)
option(OUROBOROS_TEST_ENABLED "the tests are enabled"           ON)

if (OUROBOROS_USE_CLANG)
    set(CMAKE_CXX_COMPILER clang++)
    set(CMAKE_C_COMPILER clang)
endif (OUROBOROS_USE_CLANG)

if (CMAKE_EXPORT_COMPILE_COMMANDS)
   add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON) 
endif (CMAKE_EXPORT_COMPILE_COMMANDS)

find_package(Boost COMPONENTS system filesystem regex date_time unit_test_framework REQUIRED)
if (Boost_FOUND)
    add_definitions(-DOUROBOROS_BOOST_ENABLED)
    include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

if (OUROBOROS_USE_CLANG)
    set(CMAKE_CXX_FLAGS "-std=c++98 -Weverything -Wformat -Wformat-security -Werror")
else (OUROBOROS_USE_CLANG)
    set(CMAKE_CXX_FLAGS "--std=c++98 -Wall -Wextra -Wformat -Wformat-security -Werror")
endif (OUROBOROS_USE_CLANG)
add_subdirectory(ouroboros)
if (OUROBOROS_EXAMPLES_ENABLED)
    add_subdirectory(examples)
endif (OUROBOROS_EXAMPLES_ENABLED)
if (OUROBOROS_TOOLS_ENABLED)
    add_subdirectory(tools)
endif (OUROBOROS_TOOLS_ENABLED)
if (Boost_FOUND AND OUROBOROS_TEST_ENABLED)
    enable_testing()
    add_subdirectory(tests)
endif (Boost_FOUND AND OUROBOROS_TEST_ENABLED)
