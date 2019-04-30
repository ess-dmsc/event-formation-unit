#=============================================================================
# Generate a loadable detector module
#=============================================================================
function(create_module module_name)
  add_library(${module_name} MODULE
    ${${module_name}_SRC}
    ${${module_name}_INC})
  set_target_properties(${module_name} PROPERTIES PREFIX "")
  set_target_properties(${module_name} PROPERTIES SUFFIX ".so")
  target_link_libraries(${module_name} efu
    ${${module_name}_LIB}
  ${EFU_COMMON_LIBS})
  if(${CMAKE_COMPILER_IS_GNUCXX})
    add_linker_flags(${module_name} "-Wl,--no-as-needed")
  endif()

  if (GPERF)
    target_link_libraries(${module_name} ${GPERFTOOLS_PROFILER})
  endif()

  set_target_properties(${module_name} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules")

  enable_coverage(${module_name})
  install(TARGETS ${module_name} DESTINATION bin)
endfunction(create_module)

#=============================================================================
# Compile detector module code for static linking into the EFU.
#=============================================================================
function(create_object_module module_name)
  add_library(${module_name} OBJECT
    ${${module_name}_SRC}
    ${${module_name}_INC})
    enable_coverage(${module_name})
  set(EFU_MODULES ${EFU_MODULES} $<TARGET_OBJECTS:${module_name}> CACHE INTERNAL "EFU_MODULES")
  set(EFU_DEPENDENCIES ${EFU_DEPENDENCIES} ${module_name} CACHE INTERNAL "EFU_DEPENDENCIES")
endfunction(create_object_module)

#=============================================================================
# Generate an executable program
#=============================================================================
function(create_executable exec_name)
  add_executable(${exec_name}
    ${${exec_name}_SRC}
    ${${exec_name}_INC})

  target_link_libraries(${exec_name}
    PUBLIC ${${exec_name}_LIB}
    ${EFU_COMMON_LIBS}
    efu_common)

  if (GPERF)
    target_link_libraries(${exec_name} ${GPERFTOOLS_PROFILER})
  endif()

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
    ${GTEST_LIBRARIES} efu_common)

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
    efu_common)
endfunction(create_integration_test_executable)
