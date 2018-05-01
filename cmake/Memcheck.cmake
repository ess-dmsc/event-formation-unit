function(setup_memcheck results_dir)
  find_program(VALGRIND_CMD valgrind)
  mark_as_advanced(VALGRIND_CMD)
  if(EXISTS ${VALGRIND_CMD})
    file(MAKE_DIRECTORY ${results_dir})
    set(VALGRIND_RESULTS_DIR ${results_dir} PARENT_SCOPE)
    set(VALGRIND_ENABLED ON PARENT_SCOPE)
    message(STATUS "Valgrind found. Configureed memory checks. Results dir: ${results_dir}")
  else()
    set(VALGRIND_ENABLED OFF PARENT_SCOPE)
    message(STATUS "Valgrind not found. Unable to configure memory check.")
  endif()
endfunction()

set(vagrind_targets "" CACHE INTERNAL "All valgrind targets")

function(memcheck_test test_target test_binary_dir)
  if(${VALGRIND_ENABLED})
    add_custom_target(memcheck_${test_target} COMMAND ${VALGRIND_CMD}
        --tool=memcheck --leak-check=full --verbose --xml=yes
        --xml-file=${VALGRIND_RESULTS_DIR}/${test_target}_memcheck.valgrind
        ${test_binary_dir}/${test_target})
    set(valgrind_targets
        ${valgrind_targets} memcheck_${test_target}
        CACHE INTERNAL "All valgrind targets")
  endif()
endfunction()
