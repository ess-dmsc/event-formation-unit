
option(GOOGLE_BENCHMARK "Enable google benchmark for unit tests" OFF)

set(benchmark_targets "" CACHE INTERNAL "All targets")

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

    target_link_libraries(${exec_name}
      ${${exec_name}_LIB}
      ${link_libraries}
      ${EFU_COMMON_LIBS}
      ${GTEST_LIBRARIES}
      ${CMAKE_THREAD_LIBS_INIT}
      -lbenchmark -lpthread efu_common)
      
    set(benchmark_targets
      ${exec_name}
      ${benchmark_targets}
      CACHE INTERNAL "All targets")
  else()
    message(STATUS "*** Skipping benchmark for ${exec_name} (can be enabled by cmake -DGOOGLE_BENCHMARK=YES)")
  endif()
endfunction(create_benchmark_executable)
