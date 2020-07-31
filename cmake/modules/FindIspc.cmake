# Very basic ISPC support
# Enables Ispc support if Ispc is found.
#
# Used in the following way:
#
# set(AdcSimulator_ISPC_OUT "")
# if(${ISPC_ENABLED})
#   list(APPEND AdcSimulator_ISPC_OUT SampleGen.ispc.o)
#   add_custom_command(OUTPUT SampleGen.ispc.o
#     COMMAND ${ISPC_CMD} --target=avx2 ${CMAKE_CURRENT_LIST_DIR}/SampleGen.ispc -o SampleGen.ispc.o
#     DEPENDS ${CMAKE_CURRENT_LIST_DIR}/SampleGen.ispc)
# endif()
#
# and then add ${AdcSimulator_ISPC_OUT} after the _SRC and _INC of your target executable:
#
# add_executable(AdcSimulator EXCLUDE_FROM_ALL ${AdcSimulator_SRC} ${AdcSimulator_INC} ${AdcSimulator_ISPC_OUT})
#
# \todo make a function that takes a source input and outputs the compiled .ispc.o file in the_ISPC_OUT list

unset(ISPC_CMD CACHE) # test add/remove

find_program(ISPC_CMD ispc)
mark_as_advanced(ISPC_CMD)
if(EXISTS ${ISPC_CMD})
  message(STATUS "ISPC compiler found: ${ISPC_CMD}")
else()  
  message(STATUS "ISPC compiler not found. Fallback back to (slow) c++.")  
endif()

function(add_ispc_support target_name)
  if(EXISTS ${ISPC_CMD})
    message(STATUS "add_ispc_support true")
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY COMPILE_FLAGS "-DBUILD_SUPPORT_ISPC")
  else()
    message(STATUS "add_ispc_support false")
  endif()
endfunction()

macro(ispc_compile_single_file target_out_obj_list ispc_source_file target_instruction_set)
  if(EXISTS ${ISPC_CMD})
    message(STATUS "ispc_compile_single_file true")
    set(out_obj_file "${ispc_source_file}.o")
    list(APPEND ${target_out_obj_list} ${out_obj_file})
    add_custom_command(OUTPUT ${out_obj_file}
        COMMAND ${ISPC_CMD} --target=${target_instruction_set} ${CMAKE_CURRENT_LIST_DIR}/${ispc_source_file} -o ${out_obj_file}
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/${ispc_source_file})
  else()
    message(STATUS "ispc_compile_single_file false")
  endif()
endmacro()