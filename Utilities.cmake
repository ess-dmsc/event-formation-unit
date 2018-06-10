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
    add_linker_flags(${module_name} "-Wl,--no-as-needed")
  endif()

  set_target_properties(${module_name} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules")

  enable_coverage(${module_name})
  install(TARGETS ${module_name} DESTINATION bin)
endfunction(create_module)

#=============================================================================
# Compile detector module code for static linking into the EFU 
# Note that you must add the compiled code to the EFU for linking using (e.g)
# target_link_libraries(efu $<TARGET_OBJECTS:SomeDetectorModule>)
#=============================================================================
function(create_object_module module_name)
  add_library(${module_name} OBJECT
    ${${module_name}_SRC}
    ${${module_name}_INC})
endfunction(create_object_module)

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

  set_target_properties(${exec_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

  enable_coverage(${exec_name})
  install(TARGETS ${exec_name} DESTINATION bin)
endfunction(create_executable)


#=============================================================================
# Generate unit test targets
#=============================================================================
set(unit_test_targets "" CACHE INTERNAL "All test targets")

function(create_test_executable)
  set(options SKIP_MEMGRIND)
  set(oneValueArgs "")
  set(multiValueArgs "")
  cmake_parse_arguments(create_test_executable "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(exec_name ${create_test_executable_UNPARSED_ARGUMENTS})

  add_executable(${exec_name} EXCLUDE_FROM_ALL
    ${${exec_name}_SRC}
    ${${exec_name}_INC})
  target_include_directories(${exec_name}
    PRIVATE ${GTEST_INCLUDE_DIRS})

  target_link_libraries(${exec_name}
    ${${exec_name}_LIB}
    ${EFU_COMMON_LIBS}
    ${GTEST_LIBRARIES})

  if(${CMAKE_COMPILER_IS_GNUCXX})
    add_linker_flags(${exec_name} "-Wl,--no-as-needed")
  endif()

  set_target_properties(${exec_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/unit_tests")

  add_test(NAME regular_${exec_name}
    COMMAND ${exec_name}
    "--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${exec_name}test.xml")

  set(unit_test_targets
    ${unit_test_targets} ${exec_name}
    CACHE INTERNAL "All test targets")

  enable_coverage(${exec_name})
  if(NOT ${create_test_executable_SKIP_MEMGRIND})
    memcheck_test(${exec_name} ${CMAKE_BINARY_DIR}/unit_tests)
  endif()
endfunction(create_test_executable)

function(create_integration_test_executable exec_name)
  add_executable(${exec_name} EXCLUDE_FROM_ALL
    ${${exec_name}_SRC}
    ${${exec_name}_INC})
  target_include_directories(${exec_name}
    PRIVATE ${GTEST_INCLUDE_DIRS})

  set_target_properties(${exec_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/integration_tests")

  target_link_libraries(${exec_name}
    ${${exec_name}_LIB}
    ${EFU_COMMON_LIBS}
    eventlib)
endfunction(create_integration_test_executable)
