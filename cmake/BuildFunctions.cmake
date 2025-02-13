cmake_minimum_required(VERSION 3.10)
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

#=============================================================================
# Generate a loadable detector module
#=============================================================================
function(create_module module_name)
  add_library(${module_name} MODULE
    ${${module_name}_SRC}
    ${${module_name}_INC})

  if (CMAKE_BUILD_TYPE STREQUAL "RELEASE" OR CMAKE_BUILD_TYPE STREQUAL "Release")
    if( lto_supported )
        message(STATUS "LTO enabled for ${module_name}")
        set_property(TARGET ${module_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "LTO not supported (for target ${module_name}): <${lto_error}>")
    endif()
  endif ()

  set_target_properties(${module_name} PROPERTIES PREFIX "")
  set_target_properties(${module_name} PROPERTIES SUFFIX ".so")
  target_link_libraries(${module_name} efu
    ${${module_name}_LIB}
    ${EFU_COMMON_LIBS}
    )
  if(${CMAKE_COMPILER_IS_GNUCXX})
    add_linker_flags(${module_name} "-Wl,--no-as-needed")
  endif()

  if(GPERF)
    target_link_libraries(${module_name} ${GPERFTOOLS_PROFILER})
  endif()

  set_target_properties(${module_name} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules")

  enable_coverage(${module_name})
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
    efu_common)

  if(GPERF)
    target_link_libraries(${exec_name} ${GPERFTOOLS_PROFILER})
  endif()

  set_target_properties(${exec_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

  enable_coverage(${exec_name})
endfunction(create_executable)

#=============================================================================
# Install a Python file structure under runtime output directory
#=============================================================================
function(install_python_executable python_exec_name)

  # Retrieve the list of Python files
  set(python_files ${${python_exec_name}_PY})

  # Create a custom target for installing all Python scripts
  add_custom_target(${python_exec_name} ALL)

  # Iterate over each Python file
  foreach(python_file IN LISTS python_files)
    # Calculate the relative directory path for the file
    get_filename_component(file_dir ${python_file} DIRECTORY)
    # Calculate the output directory path
    set(output_dir "$<TARGET_PROPERTY:${python_exec_name},RUNTIME_OUTPUT_DIRECTORY>/${file_dir}")

    # Add custom command for copying and setting permissions for each file
    add_custom_command(TARGET ${python_exec_name} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${output_dir}
      COMMAND ${CMAKE_COMMAND} -E copy 
        ${CMAKE_CURRENT_SOURCE_DIR}/${python_file} 
        ${output_dir}
      # Use execute_process to call chmod directly
      COMMAND ${CMAKE_COMMAND} -E env chmod 755 ${output_dir}/${python_file}
      COMMENT "Installing Python executable: ${python_file}"
    )
  endforeach()
endfunction(install_python_executable)

#=============================================================================
# Generate unit test targets
#=============================================================================
set(unit_test_targets "" CACHE INTERNAL "All test targets")

#
#
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
    PRIVATE ${GTEST_INCLUDE_DIRS}
    )

  set_property(TARGET ${exec_name}
    APPEND_STRING PROPERTY COMPILE_FLAGS "-DBUILD_IS_TEST_ENVIRONMENT")

  target_link_libraries(${exec_name}
    ${${exec_name}_LIB}
    ${EFU_COMMON_LIBS}
    ${CONAN_LIBS_GTEST} efu_common efu_reduction efu_essreadout TestUtilsLib)

  if(${CMAKE_COMPILER_IS_GNUCXX})
    add_linker_flags(${exec_name} "-Wl,--no-as-needed")
  endif()

  set_target_properties(${exec_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/unit_tests")

  add_test(NAME regular_${exec_name}
    COMMAND ${exec_name} --rerun-failed --output-on-failure
    "--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${exec_name}test.xml")

  set(unit_test_targets
    ${unit_test_targets} ${exec_name}
    CACHE INTERNAL "All test targets")

  enable_coverage(${exec_name})
  if(NOT ${create_test_executable_SKIP_MEMGRIND})
    memcheck_test(${exec_name} ${CMAKE_BINARY_DIR}/unit_tests)
  endif()
endfunction(create_test_executable)

#
#
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
