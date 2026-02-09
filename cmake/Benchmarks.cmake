cmake_minimum_required(VERSION 3.10)
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

option(GOOGLE_BENCHMARK "Enable google benchmark for unit tests" OFF)

set(benchmark_targets "" CACHE INTERNAL "All targets")
set(benchmark_executables "" CACHE INTERNAL "All benchmark executables")

find_package(ValgrindSdk)

#
# Generate benchmark targets
#
function(create_benchmark_executable exec_name)
  if(GOOGLE_BENCHMARK)
    add_executable(${exec_name} EXCLUDE_FROM_ALL
      ${${exec_name}_SRC}
      ${${exec_name}_INC})
    target_include_directories(${exec_name}
      PRIVATE ${GTEST_INCLUDE_DIRS})
    set_target_properties(${exec_name} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")

    add_valgrind_sdk (${exec_name})

    # Force optimization for benchmarks even in Debug builds
    # This ensures accurate performance measurements
    if (CMAKE_BUILD_TYPE STREQUAL "DEBUG" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
      message(STATUS "Benchmark ${exec_name}: Overriding Debug build with -O3 optimization")
      target_compile_options(${exec_name} PRIVATE -O3 -DNDEBUG)
    endif()

    # TODO investigate why LTO doesn't seem to have any effect.
    if (CMAKE_BUILD_TYPE STREQUAL "RELEASE" OR CMAKE_BUILD_TYPE STREQUAL "Release")
      if( lto_supported )
          message(STATUS "LTO enabled for ${exec_name}")
          set_property(TARGET ${exec_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
      else()
          message(STATUS "LTO not supported (for target ${exec_name}): <${lto_error}>")
      endif()
    endif ()

    target_link_libraries(${exec_name}
      PRIVATE
        ${${exec_name}_LIB}
        ${link_libraries}
        ${EFU_COMMON_LIBS}
        ${GTEST_LIBRARIES}
        benchmark::benchmark
        ${CMAKE_THREAD_LIBS_INIT}
        -lpthread efu_common efu_reduction
    )

    set(benchmark_targets
      ${exec_name}
      ${benchmark_targets}
      CACHE INTERNAL "All targets")
    
    set(benchmark_executables
      ${CMAKE_BINARY_DIR}/benchmarks/${exec_name}
      ${benchmark_executables}
      CACHE INTERNAL "All benchmark executables")
  else()
    message(STATUS "ECDC: Skipping benchmark for ${exec_name} (can be enabled by cmake -DGOOGLE_BENCHMARK=YES)")
  endif()
endfunction(create_benchmark_executable)

# ========================
# Benchmark report targets
# ========================

# Ensure benchmark output directory exists
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark_results)

# Target to run all benchmarks
add_custom_target(run_benchmarks
  DEPENDS ${benchmark_targets}
  COMMENT "Running all benchmark tests"
)

# This function should be called after all benchmarks have been registered
# It creates the benchmark_report target
function(finalize_benchmarks)
  # Add custom target to run benchmarks and generate report
  add_custom_target(benchmark_report
    COMMAND ${CMAKE_SOURCE_DIR}/utils/citools/benchmark_run.sh ${CMAKE_BINARY_DIR}/benchmarks ${CMAKE_BINARY_DIR}/benchmark_results 3
    DEPENDS ${benchmark_targets}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all benchmarks and generating report"
    VERBATIM
  )
endfunction()
