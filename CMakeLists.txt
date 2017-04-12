cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("event-formation-unit")
set(event-formation-unit_VERSION_MAJOR 1)
set(event-formation-unit_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

set(EXTRA_CXX_FLAGS -std=c++11 -pthread -fPIC)
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-D__FAVOR_BSD") #Not working correctly
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs")

set(LINUX FALSE)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    message(STATUS "Detected MacOSX")
    
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Detected Linux")
    
    find_library(DL_LIB dl REQUIRED)
    
    set(LINUX TRUE)
else()
    message(STATUS "Unknown system")
endif()

option(COV "Enable code coverage test for unit tests (if possible)." OFF)
option(EXT_FB "Use flatbuffer headers defined in external \"streaming-data-types\" repository. That repository must be located in the root of this repository." OFF)
if(${EXTSCHEMAS})
    set(GENERAL_COMPILER_FLAGS ${EXTRA_CXX_FLAGS} "-DFLATBUFFERS")
endif()

add_definitions(${EXTRA_CXX_FLAGS})

find_package(GTest)

add_subdirectory(libs)
add_subdirectory(prototype2)

add_custom_target(hints
    COMMAND echo ""
    COMMAND echo "Gnumake hints for accessing basic functionality."
    COMMAND echo "Most of these commands must be proceeded by a \\\"make all\\\""
    COMMAND echo "command."
    COMMAND echo "---------------------------------------------------------------"
    COMMAND echo "Run prototype2 unit tests:             make runtest"
    COMMAND echo "Run libs unit tests:                   make runtest_libs"
    COMMAND echo "Run memcheck tests:                    make valgrind"
    COMMAND echo "Run coverage tests:                    make coverage"
    COMMAND echo "Verbose output:                        make VERBOSE=1"
    COMMAND echo ""
    COMMAND echo "CMake hints"
    COMMAND echo "Note: it is possible to set several options at the same time"
    COMMAND echo "e.g. \\\"cmake -DCOV=ON -DEXTSCHEMAS=OFF ..\\\"."
    COMMAND echo "---------------------------------------------------------------"
    COMMAND echo "List cmake variables:                  cmake .. -LH     "
    COMMAND echo "Enable code coverage:                  cmake -DCOV=ON .."
    COMMAND echo "Use external flatbuffer headers:       cmake -DEXTSCHEMAS=ON .."
    COMMAND echo ""
)
