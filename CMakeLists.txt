cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("event-formation-unit")
set(event-formation-unit_VERSION_MAJOR 1)
set(event-formation-unit_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug)
endif()

function(MakeBuildStr)
    execute_process(COMMAND "whoami" OUTPUT_VARIABLE user_name OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "date" "+%F %H:%M:%S" OUTPUT_VARIABLE date OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "uname" "-n" OUTPUT_VARIABLE m_name OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "uname" "-r" OUTPUT_VARIABLE m_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "git" "rev-parse" "--short" "HEAD" OUTPUT_VARIABLE hash OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(BUILDSTR "${date} [${m_name}:${user_name}] [${m_version}] ${hash}" PARENT_SCOPE)
endfunction()

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    if (NOT DEFINED BUILDSTR)
        message(FATAL_ERROR "Build string must be set manually when compiling using release mode e.g. \"-DBUILDSTR=v1.2.3\".")
    endif()
else()
    # add_custom_command(OUTPUT BUILDSTR COMMAND echo "\\\"SomeOtherVersionStr\\\"")
    # add_custom_target(dummy_target ALL DEPENDS BUILDSTR)
    set(BUILDSTR "")
    MakeBuildStr()
endif()


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

message(STATUS "Build str: ${BUILDSTR}")

set(EXTRA_CXX_FLAGS -std=c++11 -pthread -fPIC)
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-D__FAVOR_BSD") #Not working correctly
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs")
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-DBUILDSTR=${BUILDSTR}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    message(STATUS "Detected MacOSX")
    
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Detected Linux")
    
    find_library(DL_LIB dl REQUIRED)
else()
    message(STATUS "Unknown system")
endif()

option(CMAKE_BUILD_TYPE "Build type; \"Release\" or \"Debug\"." Debug)

option(COV "Enable code coverage test for unit tests (if possible)." OFF)

option(EXTSCHEMAS "Use flatbuffer headers defined in external \"streaming-data-types\" repository. That repository must be located in the root of this repository." OFF)
if(${EXTSCHEMAS})
    set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-DFLATBUFFERS")
    message(STATUS "Using external FB schema header files.")
else()
    message(STATUS "Using internal FB schema header files.")
endif()

option(TRACE "What does this option do?." OFF)
if(${TRACE})
    set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-DTRACE -Ofast -flto -O3")
    message(STATUS "TRACE activated.")
else()
    message(STATUS "TRACE de-activated.")
endif()

option(DUMPTOFILE "Dump something to file?" OFF)
if(${DUMPTOFILE})
    set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-DDUMPTOFILE")
    message(STATUS "Dumping to file.")
else()
    message(STATUS "Not dumping to file.")
endif()

option(CMAKE_BUILD_TYPE "Build in \"Release\" or \"Debug\" mode." "Release")

option(PROF "Enable profiling." OFF)
if(${PROF})
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pg)
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -pg)
    set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} -pg)
    message(STATUS "Profiling enabled.")
else()
    message(STATUS "Profiling disabled.")
endif()

set(CMAKE_C_FLAGS_RELEASE)
set(CMAKE_CXX_FLAGS_RELEASE)


add_definitions(${EXTRA_CXX_FLAGS})

find_package(GTest)

add_subdirectory(libs)
add_subdirectory(prototype2)

add_custom_target(hints
    COMMAND echo ""
    COMMAND echo "Gnumake hints for accessing basic functionality."
    COMMAND echo "Most of these commands must be proceeded by a \\\"make all\\\""
    COMMAND echo "command."
    COMMAND echo "--------------------------------------------------------------------------------"
    COMMAND echo "Run prototype2 unit tests:        make runtest"
    COMMAND echo "Run libs unit tests:              make runtest_libs"
    COMMAND echo "Run memcheck tests:               make valgrind"
    COMMAND echo "Run coverage tests:               make coverage"
    COMMAND echo "Verbose output:                   make VERBOSE=1"
    COMMAND echo ""
    COMMAND echo "CMake hints"
    COMMAND echo "Note: it is possible to set several options at the same time"
    COMMAND echo "e.g. \\\"cmake -DCOV=ON -DEXTSCHEMAS=OFF ..\\\"."
    COMMAND echo "--------------------------------------------------------------------------------"
    COMMAND echo "List cmake variables:             cmake .. -LH     "
    COMMAND echo "Enable code coverage:             cmake -DCOV=ON .."
    COMMAND echo "Use external flatbuffer headers:  cmake -DEXTSCHEMAS=ON .."
    COMMAND echo "Set build type to release:        cmake -DCMAKE_BUILD_TYPE=Release .."
    COMMAND echo "            NOTE: Relase mode requires the you supply a build string"
    COMMAND echo "                  e.g. \\\"-DBUILDSTR=v1.2.3\\\"."
    COMMAND echo "Set build type to debug:          cmake -DCMAKE_BUILD_TYPE=Debug .."
    COMMAND echo "Suggest PCAP root:                cmake -DPCAP_ROOT_DIR=/opt/dmg_grp .."
    COMMAND echo "Suggest GraylogLogger root:       cmake -DGraylogLogger_ROOT_DIR=/opt/dmg_grp .."
    COMMAND echo "Suggest LibRDKafka root:          cmake -DLibRDKafka_ROOT_DIR=/opt/dmg_grp .."
    COMMAND echo ""
)
