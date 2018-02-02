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
