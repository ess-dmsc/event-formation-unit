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
# Conan
#=============================================================================

SET(CONAN_PROFILE "default" CACHE STRING "Name of conan profile to use, uses default by default")
SET(CONAN "AUTO" CACHE STRING "conan options AUTO (conan must be in path), MANUAL (expects conanbuildinfo.cmake in build directory) or DISABLE")
if(${CONAN} MATCHES "AUTO")
  include(${CMAKE_MODULE_PATH}/conan.cmake)
  conan_cmake_run(CONANFILE conanfile.txt
      PROFILE ${CONAN_PROFILE}
      BASIC_SETUP NO_OUTPUT_DIRS KEEP_RPATHS
      BUILD_TYPE "None"
      BUILD outdated)
elseif(${CONAN} MATCHES "MANUAL")
  if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(NO_OUTPUT_DIRS SKIP_RPATH)
  else()
    MESSAGE(FATAL_ERROR "CONAN set to MANUAL but no file named conanbuildinfo.cmake found in build directory")
  endif()
elseif(NOT ${CONAN} MATCHES "DISABLE")
  MESSAGE(FATAL_ERROR "Unrecognised option for CONAN (${CONAN}), use AUTO, MANUAL or DISABLE")
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
setup_memcheck(${CMAKE_BINARY_DIR}/memcheck_res)

#=============================================================================
# The code
#=============================================================================
include_directories(.)
include(Utilities.cmake)
add_subdirectory(libs)
add_subdirectory(prototype2)
add_subdirectory(udp)
add_subdirectory(dataformats)

#=============================================================================
# Finalize tests
#=============================================================================

# Should unit_test_targets depend on default targets ???

add_custom_target(unit_tests
  DEPENDS ${unit_test_targets})
add_custom_target(runtest
    COMMAND ${CMAKE_CTEST_COMMAND} -V -R regular_*
    DEPENDS unit_tests)
add_custom_target(valgrind
    DEPENDS ${valgrind_targets})
add_custom_target(benchmark
    DEPENDS ${benchmark_targets})

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

#=============================================================================
# Hints
#=============================================================================

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
