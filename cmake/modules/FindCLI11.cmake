find_path(CLI11_ROOT_DIR
  NAMES include/CLI/CLI.hpp
)

find_path(CLI11_INCLUDE_DIR
  NAMES CLI.hpp
  HINTS ${CLI11_ROOT_DIR}/include/CLI
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
