cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project("event-formation-unit")

#=============================================================================
# Conan package location for find_package()
#=============================================================================
# This ensures CMake can find packages from CMakeDeps
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

#=============================================================================
# Augment CMake with our custom scripts
#=============================================================================
set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR}/modules)

#=============================================================================
# Set relative RPATH
#=============================================================================
if(APPLE)
  set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
else()
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

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
# General configuration
#=============================================================================
include(${EXTRA_MODULES_DIR}/EnsureBuildType.cmake)
include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/BuildString.cmake)

#=============================================================================
# Conan
#=============================================================================
# cmake -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release ..

# Use CMakeDeps to find Conan packages
# Conan packages will be discoverable via find_package()
# Example: find_package(fmt REQUIRED)

#=============================================================================
# Testing-related setup
#=============================================================================
enable_testing()
include(${EXTRA_MODULES_DIR}/BuildFunctions.cmake)
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
add_subdirectory(src)

#=============================================================================
# Finalize tests
#=============================================================================
add_custom_target(unit_tests DEPENDS ${unit_test_targets})
add_custom_target(runtest COMMAND ${CMAKE_CTEST_COMMAND} -V -R regular_* DEPENDS unit_tests)
add_custom_target(valgrind DEPENDS ${valgrind_targets})
add_custom_target(benchmark DEPENDS ${benchmark_targets})
add_custom_target(doxygen COMMAND doxygen ../src/doxyfile)

# Finalize benchmarks (creates benchmark_report target)
if(GOOGLE_BENCHMARK)
  finalize_benchmarks()
endif()

#=============================================================================
# Cppcheck
#=============================================================================
add_custom_target(cppcheck
  COMMAND cppcheck -v `../cppcheck_exclude_tests.sh ../src` --platform=unix64 --force --enable=all
          -I ../src '--template={file},{line},{severity},{id},{message}' --xml --xml-version=2 ../src
          --output-file=cppcheck.xml)

add_custom_target(cppcheckhtml
  COMMAND cppcheck-htmlreport --file cppcheck.xml --report-dir cppcheckhtml)

#=============================================================================
# Help target
#=============================================================================
add_custom_target(hints
  COMMAND echo ""
  COMMAND echo "EFU Build System Hints"
  COMMAND echo "-----------------------"
  COMMAND echo "Build unit tests:                       make unit_tests"
  COMMAND echo "Run unit tests:                         make runtest"
  COMMAND echo "Run memcheck tests:                     make valgrind"
  COMMAND echo "Run coverage tests:                     make coverage"
  COMMAND echo "Run benchmarks (with report):           make benchmark_report"
  COMMAND echo "Run cppcheck:                           make cppcheck"
  COMMAND echo "Generate cppcheck HTML report:          make cppcheckhtml"
  COMMAND echo "Configure with Release build:           cmake -DCMAKE_BUILD_TYPE=Release"
  COMMAND echo "Enable benchmarks:                      cmake -DGOOGLE_BENCHMARK=YES"
  COMMAND echo "Specify Conan toolchain:                cmake -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake"
)
