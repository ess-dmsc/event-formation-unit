cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("event-formation-unit")
set(event-formation-unit_VERSION_MAJOR 1)
set(event-formation-unit_VERSION_MINOR 0)

#=============================================================================
# Augment CMake with our custom scripts
#=============================================================================
set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR}/modules)

#=============================================================================
# In case conan has generated info, use it now
#=============================================================================
if(EXISTS ${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  message(STATUS "Using existing conanbuildinfo.cmake file")
  include(${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup(KEEP_RPATHS)
endif()

#=============================================================================
# General configuration
#=============================================================================
include(${EXTRA_MODULES_DIR}/EnsureBuildType.cmake)
include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/BuildString.cmake)

#=============================================================================
# Testing-related stuff
#=============================================================================
enable_testing()
find_package(GTestFix REQUIRED)
include(${EXTRA_MODULES_DIR}/AddTargetFlags.cmake)
include(${EXTRA_MODULES_DIR}/CoverageReports.cmake)
include(${EXTRA_MODULES_DIR}/Benchmarks.cmake)
include(${EXTRA_MODULES_DIR}/Profiling.cmake)
include(${EXTRA_MODULES_DIR}/Memcheck.cmake)

#=============================================================================
# The code
#=============================================================================
add_subdirectory(libs)
add_subdirectory(prototype2)
add_subdirectory(udp)
#add_subdirectory(dataformats) # built separately for now

#=============================================================================
# Finalize coverage
#=============================================================================
set(gcovr_excl_opts
  "-e .*Test.cpp -e .*gtest.*.h -e .*_generated.h -e .*CLI11.hpp")
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
