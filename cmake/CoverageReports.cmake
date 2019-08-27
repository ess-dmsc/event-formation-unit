#=========================================================================================
# Enabling code coverage
#=========================================================================================
#
#   1. For each target that you want to enable coverage reporting for
#      invoke enable_coverage(target_name)
#
#   2. Automatically create coverage generation targets with:
#     create_coverage_targets(target_base_name run_target bin_dir source_dir output_path)
#     where:
#       targer_base_name -- will create *_html *_xml and * which makes both
#       run_target -- name of target that runs the tests
#       bin_dir -- root directory of unit test binaries
#       source_dir -- root directory of source for which to make reports
#       output_path -- where reports should be written to
#
#   3. when building, invoke cmake with -DCOV=1 option
#

option(COV "Enable code coverage reporting for unit tests (if possible)." OFF)

function(configure_coverage_gcc)
  set(COVERAGE_COMPILE_FLAGS "-g -O0 -coverage -fprofile-generate -ftest-coverage" PARENT_SCOPE)
  set(COVERAGE_LINK_FLAGS "-coverage -fprofile-generate -ftest-coverage" PARENT_SCOPE)
  find_program(COVERAGE_GCOV_PATH gcov)
  find_program(COVERAGE_GCOVR_PATH gcovr PATHS ${CMAKE_CURRENT_LIST_DIR})
  if(NOT COVERAGE_GCOV_PATH)
    message(WARNING "Cannot enable coverage reporting; gcov not found.")
    set(COVERAGE_ENABLED OFF PARENT_SCOPE)
  elseif(NOT COVERAGE_GCOVR_PATH)
    message(WARNING "Cannot enable coverage reporting; gocvr not found.")
    set(COVERAGE_ENABLED OFF PARENT_SCOPE)
  else()
    set(COVERAGE_LIBRARIES gcov PARENT_SCOPE)
    message(STATUS "Enabled coverage reporting for GCC")
    set(COVERAGE_ENABLED ON PARENT_SCOPE)
  endif()
endfunction()

function(configure_coverage_llvm)
  set(COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-instr-generate -fcoverage-mapping" PARENT_SCOPE)
  set(COVERAGE_LINK_FLAGS "-g -O0 -fprofile-instr-generate -fcoverage-mapping" PARENT_SCOPE)
  find_program(COVERAGE_LLVM_COV_PATH NAMES "llvm-cov-mp-4.0" "llvm-cov")
  find_program(COVERAGE_LLVM_PROF_PATH NAMES "llvm-profdata-mp-4.0" "llvm-profdata")
  if(NOT COVERAGE_LLVM_PROF_PATH OR NOT COVERAGE_LLVM_COV_PATH)
    message(WARNING "Cannot enable coverage reporting; llvm-cov and/or llvm-profdata not found.")
    set(COVERAGE_ENABLED OFF PARENT_SCOPE)
  else()
    message(STATUS "Enabled coverage reporting for Clang")
    set(COVERAGE_ENABLED ON PARENT_SCOPE)
  endif()
endfunction()

if(${COV})
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    configure_coverage_llvm()
  elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    configure_coverage_gcc()
  else()
    message(WARNING "Cannot enable coverage reporting. Unsupported compiler: (${CMAKE_CXX_COMPILER_ID}).")
    set(COVERAGE_ENABLED OFF)
  endif()

  if(${COVERAGE_ENABLED})
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      message(WARNING "Cannot provide coverage reporting with Release build. Disabling coverage.")
      set(COVERAGE_ENABLED OFF)
    endif()
  endif()
endif()

function(enable_coverage_flags coverage_target)
  if(${COVERAGE_ENABLED})
    set_target_properties(${coverage_target} PROPERTIES
        COMPILE_FLAGS ${COVERAGE_COMPILE_FLAGS})
    set_target_properties(${coverage_target} PROPERTIES
        LINK_FLAGS ${COVERAGE_LINK_FLAGS})
  endif()
endfunction()

function(enable_coverage coverage_target)
  if(${COVERAGE_ENABLED})
    set_target_properties(${coverage_target} PROPERTIES
        COMPILE_FLAGS ${COVERAGE_COMPILE_FLAGS})
    set_target_properties(${coverage_target} PROPERTIES
        LINK_FLAGS ${COVERAGE_LINK_FLAGS})
    target_link_libraries(${coverage_target}
        PUBLIC ${COVERAGE_LIBRARIES})
  endif()
endfunction()

function(create_coverage_targets target_base_name run_target bin_dir source_dir output_path excl)
  if(${COVERAGE_ENABLED})
    file(MAKE_DIRECTORY ${output_path})
    separate_arguments(excl)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
      add_custom_target(${target_base_name}_html
          COMMAND ${COVERAGE_LLVM_PROF_PATH}
          "merge" "default.profraw" "-o" "default.profdata" ";" ${COVERAGE_LLVM_COV_PATH}
          "show" "${bin_dir}/bin/unit_tests" "-instr-profile=default.profdata" "-format=html" ">${output_path}/coverage.html"
          DEPENDS ${run_target})
      # ";" ${COVERAGE_LLVM_COV_PATH} "report" "${bin_dir}/lib"
      add_custom_target(${target_base_name} DEPENDS ${target_base_name}_html)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
      add_custom_target(${target_base_name}_xml

          COMMAND ${COVERAGE_GCOVR_PATH}
          "${bin_dir}"
#          "--verbose"
          "--xml"
          "--exclude-unreachable-branches"
          "--print-summary"
          "--root=${source_dir}"
          "--output=${output_path}/coverage.xml"
          ${excl}
          DEPENDS ${run_target})
      add_custom_target(${target_base_name}_html
          COMMAND ${COVERAGE_GCOVR_PATH}
          "${bin_dir}"
#          "--verbose"
          "--html"
          "--html-details"
          "--exclude-unreachable-branches"
          "--root=${source_dir}"
          "--output=${output_path}/index.html"
          ${excl}
          DEPENDS ${run_target})
      add_custom_target(${target_base_name}
          DEPENDS ${target_base_name}_xml ${target_base_name}_html)
    endif()
  endif()
endfunction()
