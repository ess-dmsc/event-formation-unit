#=============================================================================
# Generate a loadable detector module
#=============================================================================
function(create_module module_name)
  add_library(${module_name} MODULE
    ${${module_name}_SRC}
    ${${module_name}_INC})
  set_target_properties(${module_name} PROPERTIES PREFIX "")
  set_target_properties(${module_name} PROPERTIES SUFFIX ".so")
  target_link_libraries(${module_name}
    ${${module_name}_LIB}
    ${EFU_COMMON_LIBS}
    eventlib)
  if(${CMAKE_COMPILER_IS_GNUCXX})
    add_linker_flags(${module_name} "--no-as-needed")
  endif()
  enable_coverage(${module_name})
  install(TARGETS ${module_name} DESTINATION bin)
endfunction(create_module)

#=============================================================================
# Generate an executable program
#=============================================================================
function(create_executable exec_name)
  add_executable(${exec_name}
    ${${exec_name}_SRC}
    ${${exec_name}_INC})

  target_link_libraries(${exec_name}
    ${${exec_name}_LIB}
    ${EFU_COMMON_LIBS}
    eventlib)
  enable_coverage(${exec_name})
  install(TARGETS ${exec_name} DESTINATION bin)
endfunction(create_executable)


#=============================================================================
# Generate unit test targets
#=============================================================================
setup_memcheck(${CMAKE_BINARY_DIR}/memcheck_res)
set(unit_test_targets "" CACHE INTERNAL "All targets")

function(create_test_executable exec_name)
  add_executable(${exec_name} EXCLUDE_FROM_ALL
    ${${exec_name}_SRC}
    ${${exec_name}_INC})
  target_include_directories(${exec_name}
    PRIVATE ${GTEST_INCLUDE_DIRS})

  # This does not seem to work right now. Conan to blame ???
  #  set_target_properties(${exec_name} PROPERTIES
  #    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/unit_tests")

  target_link_libraries(${exec_name}
    ${${exec_name}_LIB}
    ${EFU_COMMON_LIBS}
    ${GTEST_LIBRARIES})

  add_test(NAME regular_${exec_name}
    COMMAND ${exec_name}
    "--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${exec_name}test.xml")

  set(unit_test_targets
    ${unit_test_targets} ${exec_name}
    CACHE INTERNAL "All targets")

  enable_coverage(${exec_name})
  memcheck_test(${exec_name} ${CMAKE_BINARY_DIR}/bin)

endfunction(create_test_executable)

