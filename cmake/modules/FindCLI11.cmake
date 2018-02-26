find_path(CLI11_ROOT_DIR
  NAMES include/CLI11.hpp
)

find_path(CLI11_INCLUDE_DIR
  NAMES CLI11.hpp
  HINTS ${CLI11_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CLI11 FOUND_VAR CLI11_FOUND REQUIRED_VARS
  CLI11_ROOT_DIR
  CLI11_INCLUDE_DIR
)

mark_as_advanced(
  CLI11_ROOT_DIR
  CLI11_INCLUDE_DIR
)
