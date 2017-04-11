cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("event-formation-unit")
set(event-formation-unit_VERSION_MAJOR 1)
set(event-formation-unit_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

set(GENERAL_COMPILER_FLAGS -std=c++11 -pthread -fPIC)
set(GENERAL_COMPILER_FLAGS ${GENERAL_COMPILER_FLAGS} "-D__FAVOR_BSD")

set(LINUX FALSE)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    message(STATUS "Detected MacOSX")
    
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Detected Linux")
    
    find_library(DL_LIB dl net REQUIRED)
    
    set(LINUX TRUE)
else()
    message(STATUS "Unknown system")
endif()

add_definitions(${GENERAL_COMPILER_FLAGS})

option(COV "Enable code coverage test for unit tests (if possible)." OFF)

find_package(GTest)

add_subdirectory(libs)
add_subdirectory(prototype2)

add_custom_target(hints
    COMMAND echo ""
    COMMAND echo "gnumake hints for accessing basic functionality"
    COMMAND echo "-----------------------------------------------"
    COMMAND echo "Run prototype2 unit tests:  make runtest"
    COMMAND echo "Run libs unit tests:        make runtest_libs"
    COMMAND echo "Run memcheck tests:         make valgrind"
    COMMAND echo "Verbose output:             make VERBOSE=1"
    COMMAND echo ""
    COMMAND echo "CMake hints"
    COMMAND echo "-----------------------------------------------"
    COMMAND echo "List cmake variables:       cmake .. -LH     "
    COMMAND echo "Enable code coverage:       cmake -DCOV=ON .."
    COMMAND echo ""
)
