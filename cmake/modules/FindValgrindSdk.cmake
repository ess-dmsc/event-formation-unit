# Finds the Valgrind source installation.
#
# Var      VALGRIND_SDK_ROOT     Where is valgrind source.
# Var      ValgrindSdk_FOUND     Do we have the valgrind source.
# Function add_valgrind_sdk()    Adds compile define BUILD_SUPPORT_VALGRIND_SDK to target
#                                and VALGRIND_SDK_ROOT to target include directories.

find_path(VALGRIND_SDK_ROOT valgrind/valgrind.h
  /usr/include 
  /usr/local/include 
  /usr/local/opt/ 
  ${VALGRIND_SDK_PREFIX}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ValgrindSdk DEFAULT_MSG
  VALGRIND_SDK_ROOT
)

mark_as_advanced(
  VALGRIND_SDK_ROOT
)

function(add_valgrind_sdk target_name)
  if(ValgrindSdk_FOUND)
    set_property(TARGET ${target_name} APPEND PROPERTY INCLUDE_DIRECTORIES "${VALGRIND_SDK_ROOT}")
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY COMPILE_FLAGS "-DBUILD_SUPPORT_VALGRIND_SDK")
    message(STATUS "ValgrindSdk enabled for ${target_name}")
  else()
    message(STATUS "ValgrindSdk disabled for ${target_name}")
  endif()
endfunction()