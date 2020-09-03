# Very basic ISPC support
# Enables Ispc support if Ispc is found.
#
# Used in the following way:
#
# set(myTarget_ISPC_OBJ "")
# find_package(Ispc)
# ispc_compile_single_file(myTarget_ISPC_OBJ SampleGen.ispc avx2) # avx2 instruction set support
# add_executable(myTarget ${myTarget_SRC} ${myTarget_INC} ${myTarget_ISPC_OBJ})
# add_ispc_support(myTarget)

#unset(ISPC_CMD CACHE) # test add/remove

find_program(ISPC_CMD ispc)
mark_as_advanced(ISPC_CMD)
if(EXISTS ${ISPC_CMD})
  message(STATUS "ECDC: ISPC compiler found: ${ISPC_CMD}")
else()
  message(STATUS "ECDC: ISPC compiler not found. Fallback back to (slow) c++.")
endif()

function(add_ispc_support target_name)
  if(EXISTS ${ISPC_CMD})
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY COMPILE_FLAGS "-DBUILD_SUPPORT_ISPC")
  endif()
endfunction()

macro(ispc_compile_single_file target_out_obj_list ispc_source_file target_instruction_set)
  if(EXISTS ${ISPC_CMD})
    set(out_obj_file "${ispc_source_file}.o")
    list(APPEND ${target_out_obj_list} ${out_obj_file})
    add_custom_command(OUTPUT ${out_obj_file}
        COMMAND ${ISPC_CMD} --target=${target_instruction_set} ${CMAKE_CURRENT_LIST_DIR}/${ispc_source_file} -o ${out_obj_file}
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/${ispc_source_file})
  endif()
endmacro()
