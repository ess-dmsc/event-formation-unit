cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("event-formation-unit")
set(event-formation-unit_VERSION_MAJOR 1)
set(event-formation-unit_VERSION_MINOR 0)

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
set(CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})
include(${EXTRA_MODULES_DIR}/CoverageReports.cmake)

#=============================================================================
# setting up conan for use from within cmake
#=============================================================================
set(WITH_CONAN OFF CACHE BOOL "Enable dependency build with conan")
set(CONAN_FILE "conanfile.txt" CACHE STRING "The conanfile to use for the build")

if (WITH_CONAN)
  include(${EXTRA_MODULES_DIR}/ConanSetup.cmake)
  conan_cmake_run(CONANFILE ${CONAN_FILE} 
          BASIC_SETUP
          BUILD missing)
endif ()

if (EXISTS ${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  message(STATUS "Using existing conanbuildinfo.cmake file")
  include(${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup(SKIP_RPATH)
endif ()


if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug)
endif ()

function(makebuildstr)
  execute_process(COMMAND "whoami" OUTPUT_VARIABLE user_name OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND "date" "+%F %H:%M:%S" OUTPUT_VARIABLE date OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND "uname" "-n" OUTPUT_VARIABLE m_name OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND "uname" "-r" OUTPUT_VARIABLE m_version OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND "git" "rev-parse" "--short" "HEAD" OUTPUT_VARIABLE hash OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(BUILDSTR "${date} [${m_name}:${user_name}] [${m_version}] ${hash}" PARENT_SCOPE)
endfunction()

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  if (NOT DEFINED BUILDSTR)
    message(FATAL_ERROR "Build string must be set manually when compiling using release mode e.g. \"-DBUILDSTR=v1.2.3\".")
  endif ()
else ()
  set(BUILDSTR "")
  makebuildstr()
endif ()


message(STATUS "Build str: ${BUILDSTR}")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -flto -O3 -DRELEASE -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -ggdb -flto -fno-omit-frame-pointer -O3 -DRELEASE -DNDEBUG")

option(GOOGLE_BENCHMARK, "Enable google benchmark for unit tests" OFF)

option(USE_OLD_ABI "Sets _GLIBCXX_USE_CXX11_ABI=0 for CentOS builds" ON)
set(EXTRA_CXX_FLAGS -std=c++11 -pthread -fPIC)
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-D__FAVOR_BSD") #Not working correctly
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs")
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-DBUILDSTR=${BUILDSTR}")

if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
  # Hackity-hack...
  message(STATUS "Disabling some of the newer compiler features... Hackity-hack...")
  set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-Wno-implicit-fallthrough -Wno-aligned-new")
endif ()

if (${USE_OLD_ABI})
  message(WARNING "Setting _GLIBCXX_USE_CXX11_ABI=0 consider changing this when gcc on CentOS is updated")
  set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-D_GLIBCXX_USE_CXX11_ABI=0")
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  message(STATUS "Detected MacOSX")

elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(STATUS "Detected Linux")
  find_library(DL_LIB dl REQUIRED)
else ()
  message(FATAL_ERROR "Unknown system")
endif ()

option(CMAKE_BUILD_TYPE "Build type; \"Release\" or \"Debug\"." Debug)

option(EXTSCHEMAS "Use flatbuffer headers defined in external \"streaming-data-types\" repository. That repository must be located in the root of this repository." OFF)
if (${EXTSCHEMAS})
  set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-DFLATBUFFERS")
  message(STATUS "Using external FB schema header files.")
else ()
  message(STATUS "Using internal FB schema header files.")
endif ()

option(DUMPTOFILE "Dump vmm2srsdaq data to file (Doro option)" OFF)
if (${DUMPTOFILE})
  set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-DDUMPTOFILE")
  message(STATUS "Dumping to file.")
else ()
  message(STATUS "Not dumping to file.")
endif ()

option(PROF "Enable profiling." OFF)
if (${PROF})
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pg)
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -pg)
  set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} -pg)
  message(STATUS "Profiling enabled.")
else ()
  message(STATUS "Profiling disabled.")
endif ()

add_definitions(${EXTRA_CXX_FLAGS})

include(${EXTRA_MODULES_DIR}/FindGTestFix.cmake)

if (${CMAKE_COMPILER_IS_GNUCXX})
  set(CMAKE_AR "gcc-ar")
  set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_CXX_ARCHIVE_FINISH true)
endif ()


add_subdirectory(libs)
add_subdirectory(prototype2)
add_subdirectory(udp)
#add_subdirectory(dataformats) # built separately for now

set(gcovr_excl_opts "-e .*Test.cpp -e .*gtest.*.h -e .*_generated.h" )
create_coverage_targets(
        coverage
        runtest
        ${PROJECT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/
        ${PROJECT_BINARY_DIR}/coverage
        ${gcovr_excl_opts})

add_custom_target(hints
                  COMMAND echo ""
                  COMMAND echo "GNU Make hints for accessing basic functionality."
                  COMMAND echo "Most of these commands must be preceeded by a \\\"make all\\\""
                  COMMAND echo "command."
                  COMMAND echo "--------------------------------------------------------------------------------"
                  COMMAND echo "Run unit tests:                         make runtest"
                  COMMAND echo "Run memcheck  tests:                    make valgrind"
                  COMMAND echo "Run coverage tests:                     make coverage"
                  COMMAND echo "Verbose output:                         make VERBOSE=1"
                  COMMAND echo ""
                  COMMAND echo "CMake hints"
                  COMMAND echo "--------------------------------------------------------------------------------"
                  COMMAND echo "Enable code coverage:                   cmake -DCOV=ON .."
                  COMMAND echo "Use external flatbuffer headers:        cmake -DEXTSCHEMAS=ON .."
                  COMMAND echo "Specify a build string:                 cmake -DBUILDSTR=v1.2.3 .."
                  COMMAND echo "Set build type to debug - default:      cmake -DCMAKE_BUILD_TYPE=Debug .."
                  COMMAND echo "Set build type to release:              cmake -DCMAKE_BUILD_TYPE=Release .."
                  COMMAND echo "    NOTE: Relase mode requires a build string"
                  COMMAND echo ""
                  COMMAND echo "CMake hints advanced/not generally used"
                  COMMAND echo "--------------------------------------------------------------------------------"
                  COMMAND echo "Suggest PCAP root:                      cmake -DPCAP_ROOT_DIR=/opt/dmg_grp .."
                  COMMAND echo "Suggest GraylogLogger root:             cmake -DGraylogLogger_ROOT_DIR=/opt/dmg_grp .."
                  COMMAND echo "Suggest LibRDKafka root:                cmake -DLibRDKafka_ROOT_DIR=/opt/dmg_grp .."
                  COMMAND echo "Dump vmm readout to file - det. group:  cmake -DDUMPTOFILE=ON .."
                  COMMAND echo "List cmake variables:                   cmake .. -LH     "
                  COMMAND echo ""
                  )
