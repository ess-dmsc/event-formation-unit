find_path(Fakeit_ROOT_DIR
  NAMES include/fakeit.hpp
)

find_path(Fakeit_INCLUDE_DIR
  NAMES fakeit.hpp
  HINTS ${Fakeit_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Fakeit FOUND_VAR Fakeit_FOUND REQUIRED_VARS
  Fakeit_ROOT_DIR
  Fakeit_INCLUDE_DIR
)

mark_as_advanced(
  Fakeit_ROOT_DIR
  Fakeit_INCLUDE_DIR
)

