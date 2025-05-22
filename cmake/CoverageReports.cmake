# =========================================================================================
# CoverageReports.cmake - Enable and generate code coverage reports using gcov + gcovr
# =========================================================================================
#
# USAGE:
#   1. Enable with:         cmake -DCOV=ON ..
#   2. Mark targets:        enable_coverage(target_name)
#   3. Build:               make runtests
#   4. Report (text):       make coverage
#   5. Report (XML):        make coverage_xml
#
# REQUIREMENTS:
#   - gcov (e.g., from gcc or llvm)
#   - gcovr (https://github.com/gcovr/gcovr)
#
# NOTES:
#   - Release builds are excluded to avoid optimized-away code.
#   - Handles parse errors like GCC bug 68080 via gcovr flag.

option(COV "Enable code coverage reporting for unit tests (if possible)." OFF)

# Global variables for the rest of the build system
set(COVERAGE_ENABLED OFF CACHE INTERNAL "Is coverage enabled")

if(COV)
  # Use gcov for coverage
  set(COVERAGE_COMPILE_FLAGS "-g -O0 -coverage -fprofile-generate -ftest-coverage")
  set(COVERAGE_LINK_FLAGS "-coverage -fprofile-generate -ftest-coverage")

  # Locate required tools
  find_program(COVERAGE_BIN gcov)
  find_program(GCOVR_BIN gcovr)

  if(NOT COVERAGE_BIN)
    message(WARNING "Cannot enable coverage reporting; gcov not found.")
  elseif(NOT GCOVR_BIN)
    message(WARNING "Cannot enable coverage reporting; gcovr not found.")
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(WARNING "Coverage not supported with Release build. Disable optimizations to continue.")
  else()
    message(STATUS "Enabled coverage reporting with gcov and gcovr")
    set(COVERAGE_ENABLED ON)
  endif()
endif()

# Apply compile/link flags to a target (used by enable_coverage)
function(enable_coverage_flags coverage_target)
  if(COVERAGE_ENABLED)
    set_target_properties(${coverage_target} PROPERTIES
      COMPILE_FLAGS "${COVERAGE_COMPILE_FLAGS}"
      LINK_FLAGS "${COVERAGE_LINK_FLAGS}"
    )
  endif()
endfunction()

# Full coverage target setup: flags + linking to gcov
function(enable_coverage coverage_target)
  if(COVERAGE_ENABLED)
    enable_coverage_flags(${coverage_target})

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_link_libraries(${coverage_target} PRIVATE gcov)
    endif()

  endif()
endfunction()

# ========================
# Coverage report targets
# ========================

# Ensure coverage output directory exists
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/coverage)

# Plain text report
add_custom_target(coverage
  COMMAND ${GCOVR_BIN}
    -r ${CMAKE_SOURCE_DIR}
    --object-directory ${CMAKE_BINARY_DIR}
    --txt
    --output ${CMAKE_BINARY_DIR}/coverage/coverage.txt
    --gcov-ignore-parse-errors=negative_hits.warn
    --exclude='.*gtest.*'
    --exclude='.*CLI.hpp'
    --exclude='.*Test.cpp'
    --exclude='.*\.conan'
  COMMAND ${CMAKE_COMMAND} -E echo "==== Coverage Summary ===="
  COMMAND tail -n 3 ${CMAKE_BINARY_DIR}/coverage/coverage.txt
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating plain text coverage report"
)

# XML report (for CI or tooling)
add_custom_target(coverage_xml
  COMMAND ${GCOVR_BIN}
    -r ${CMAKE_SOURCE_DIR}
    --object-directory ${CMAKE_BINARY_DIR}
    --cobertura-pretty 
    --cobertura
    --output ${CMAKE_BINARY_DIR}/coverage/coverage.xml
    --gcov-ignore-parse-errors=negative_hits.warn
    --exclude='.*gtest.*'
    --exclude='.*CLI.hpp'
    --exclude='.*Test.cpp'
    --exclude='.*\.conan'
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating XML coverage report"
)

# HTML report (human-friendly)
add_custom_target(coverage_html
  COMMAND ${GCOVR_BIN}
    -r ${CMAKE_SOURCE_DIR}
    --object-directory ${CMAKE_BINARY_DIR}
    --html
    --html-details
    --output ${CMAKE_BINARY_DIR}/coverage/coverage.html
    --gcov-ignore-parse-errors=negative_hits.warn
    --exclude='.*gtest.*'
    --exclude='.*CLI.hpp'
    --exclude='.*Test.cpp'
    --exclude='.*\.conan'
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating HTML coverage report"
)

add_custom_target(coverage_all
  DEPENDS coverage coverage_xml coverage_html
  COMMENT "Generating all coverage report formats (txt, xml, html)"
)
