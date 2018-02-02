setup_memcheck(${CMAKE_BINARY_DIR}/memcheck_res)

#=============================================================================
# Add linker flags
#=============================================================================
function(add_linker_flags target flags)
  get_target_property(cur_link_flags ${target} LINK_FLAGS)
  if(NOT cur_link_flags)
    set(cur_link_flags "")
  endif()
  set(new_link_flags "${cur_link_flags} ${flags}")
  set_target_properties(${target} PROPERTIES LINK_FLAGS "${new_link_flags}")
endfunction(add_linker_flags)

#=============================================================================
# Add compiler flags
#=============================================================================
function(add_compile_flags target flags)
  get_target_property(cur_compile_flags ${target} COMPILE_FLAGS)
  if(NOT cur_compile_flags)
    set(cur_compile_flags "")
  endif()
  set(new_compile_flags "${cur_compile_flags} ${flags}")
  set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${new_compile_flags}")
endfunction(add_compile_flags)

#=============================================================================
# Generate a loadable detector module
#=============================================================================
function(create_module module_name link_libraries)
  add_library(${module_name} MODULE
    ${${module_name}_SRC}
    ${${module_name}_INC})
  set_target_properties(${module_name} PROPERTIES PREFIX "")
  set_target_properties(${module_name} PROPERTIES SUFFIX ".so")
  target_link_libraries(${module_name}
    ${EFU_COMMON_LIBS}
    ${link_libraries}
    eventlib)
  enable_coverage(${module_name})
  install(TARGETS ${module_name} DESTINATION bin)
endfunction(create_module)

#=============================================================================
# Generate an executable program
#=============================================================================
function(create_executable exec_name link_libraries)
  add_executable(${exec_name}
    ${${exec_name}_SRC}
    ${${exec_name}_INC})

  target_link_libraries(${exec_name}
    ${EFU_COMMON_LIBS}
    ${link_libraries}
    eventlib)
  enable_coverage(${exec_name})
  install(TARGETS ${exec_name} DESTINATION bin)
endfunction(create_executable)


#=============================================================================
# Generate unit test targets
#=============================================================================
set(unit_test_targets "" CACHE INTERNAL "All targets")

function(create_test_executable exec_name link_libraries)
  add_executable(${exec_name} EXCLUDE_FROM_ALL
    ${${exec_name}_SRC}
    ${${exec_name}_INC})
  target_include_directories(${exec_name}
    PRIVATE ${GTEST_INCLUDE_DIRS})

  # This does not seem to work right now. Conan to blame?
  #  set_target_properties(${exec_name} PROPERTIES
  #    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/unit_tests")

  target_link_libraries(${exec_name}
    ${link_libraries}
    ${EFU_COMMON_LIBS}
    ${GTEST_LIBRARIES}
    )

  add_test(NAME regular_${exec_name}
    COMMAND ${exec_name}
    "--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${exec_name}test.xml")

  set(unit_test_targets
    ${unit_test_targets} ${exec_name}
    CACHE INTERNAL "All targets")

  enable_coverage(${exec_name})
  memcheck_test(${exec_name} ${CMAKE_BINARY_DIR}/bin)

endfunction(create_test_executable)

