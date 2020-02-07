cmake_minimum_required(VERSION 3.9.4)
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

option(GOOGLE_BENCHMARK "Enable google benchmark for unit tests" OFF)

set(benchmark_targets "" CACHE INTERNAL "All targets")


set(callgrind_INCLUDE_DIR /usr/local/opt/valgrind/include/)
set(callgrind_USING_DEFINE -DBUILD_USE_VALGRIND)
if(NOT EXISTS ${callgrind_INCLUDE_DIR})
  set(callgrind_INCLUDE_DIR )
  set(callgrind_USING_DEFINE )
endif()

message(STATUS "+++ valgrind +++: ${callgrind_INCLUDE_DIR}")

#
# Generate benchmark targets
#
function(create_benchmark_executable exec_name)
  if(GOOGLE_BENCHMARK)
    add_executable(${exec_name} EXCLUDE_FROM_ALL
      ${${exec_name}_SRC}
      ${${exec_name}_INC})
    target_include_directories(${exec_name}
      PRIVATE ${GTEST_INCLUDE_DIRS} ${callgrind_INCLUDE_DIR})
    set_target_properties(${exec_name} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")

    if (CMAKE_BUILD_TYPE STREQUAL "RELEASE" OR CMAKE_BUILD_TYPE STREQUAL "Release")
      if( lto_supported )
          message(STATUS "LTO enabled for ${exec_name}")
          set_property(TARGET ${exec_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
      else()
          message(STATUS "LTO not supported (for target ${exec_name}): <${lto_error}>")
      endif()
    endif ()

    target_link_libraries(${exec_name}
      ${${exec_name}_LIB}
      ${link_libraries}
      ${EFU_COMMON_LIBS}
      ${GTEST_LIBRARIES}
      ${CMAKE_THREAD_LIBS_INIT}
      -g -lbenchmark -lpthread efu_common)
      
    set(benchmark_targets
      ${exec_name}
      ${benchmark_targets}
      CACHE INTERNAL "All targets")
  else()
    message(STATUS "*** Skipping benchmark for ${exec_name} (can be enabled by cmake -DGOOGLE_BENCHMARK=YES)")
  endif()
endfunction(create_benchmark_executable)
