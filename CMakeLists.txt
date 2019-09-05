cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("event-formation-unit")

#=============================================================================
# Augment CMake with our custom scripts
#=============================================================================
set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR}/modules)

#=============================================================================
# Versioning
#=============================================================================
include(${EXTRA_MODULES_DIR}/Versioning.cmake)

set_version()
set(event-formation-unit_VERSION_MAJOR "${MAJOR_VERSION}")
set(event-formation-unit_VERSION_MINOR "${MINOR_VERSION}")
set(VERSION_INCLUDE_DIR ${CMAKE_BINARY_DIR}/version)
create_version_header(
  ${EXTRA_MODULES_DIR}/templates/version_num.h.in
  ${VERSION_INCLUDE_DIR}/common/version_num.h
)
create_version_header(
  ${EXTRA_MODULES_DIR}/templates/Version.h.in
  ${VERSION_INCLUDE_DIR}/common/Version.h
)

#=============================================================================
# Conan
#=============================================================================

SET(CONAN_PROFILE "default" CACHE STRING "Name of conan profile to use, uses default by default")
SET(CONAN "AUTO" CACHE STRING "conan options AUTO (conan must be in path), MANUAL (expects conanbuildinfo.cmake in build directory) or DISABLE")
if(${CONAN} MATCHES "AUTO")
  include(${CMAKE_MODULE_PATH}/conan.cmake)
  conan_cmake_run(CONANFILE conanfile.txt
      PROFILE ${CONAN_PROFILE}
      BASIC_SETUP NO_OUTPUT_DIRS KEEP_RPATHS NO_IMPORTS
      BUILD_TYPE "None"
      BUILD outdated)
elseif(${CONAN} MATCHES "MANUAL")
  if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)
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
set(REFDATA "$ENV{HOME}/ownCloud/DM/data/EFU_reference"
  CACHE STRING "Reference data root directory")
message(STATUS "Reference data root directory: ${REFDATA}")

#=============================================================================
# The code
#=============================================================================
include_directories(.)

include(Utilities.cmake)
add_subdirectory(src)
add_subdirectory(utils/udp)

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
add_custom_target(doxygen
    COMMAND doxygen ../src/doxyfile)

#=============================================================================
# Finalize coverage
#=============================================================================
set(gcovr_excl_opts
"--exclude='/.*Test.cpp' --exclude='.*gtest.*.h' --exclude='.*CLI.hpp'")
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
  COMMAND echo "Enable code coverage:                   cmake -DCOV=ON"
  COMMAND echo "Specify root for reference data:        cmake -DREFDATA=/home/username/refdata"
  COMMAND echo "Enable profiling with google perftools  cmake -DGPERF=ON"
  COMMAND echo "Specify a build string:                 cmake -DBUILDSTR=v1.2.3"
  COMMAND echo "Set build type to debug - default:      cmake -DCMAKE_BUILD_TYPE=Debug"
  COMMAND echo "Set build type to release:              cmake -DCMAKE_BUILD_TYPE=Release"
  COMMAND echo "    NOTE: Relase mode requires a build string"
  COMMAND echo ""
  COMMAND echo "CMake hints advanced/not generally used"
  COMMAND echo "--------------------------------------------------------------------------------"
  COMMAND echo "Suggest PCAP root:                      cmake -DPCAP_ROOT_DIR=/opt/dmg_grp"
  COMMAND echo "Suggest GraylogLogger root:             cmake -DGraylogLogger_ROOT_DIR=/opt/dmg_grp"
  COMMAND echo "Suggest LibRDKafka root:                cmake -DLibRDKafka_ROOT_DIR=/opt/dmg_grp"
  COMMAND echo "List cmake variables:                   cmake -LH"
  COMMAND echo ""
  )
