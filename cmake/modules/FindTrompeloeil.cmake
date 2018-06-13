find_path(Trompeloeil_ROOT_DIR
  NAMES include/trompeloeil.hpp
)

find_path(Trompeloeil_INCLUDE_DIR
  NAMES trompeloeil.hpp
  HINTS ${Trompeloeil_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Trompeloeil FOUND_VAR Trompeloeil_FOUND REQUIRED_VARS
  Trompeloeil_ROOT_DIR
  Trompeloeil_INCLUDE_DIR
)

mark_as_advanced(
  Trompeloeil_ROOT_DIR
  Trompeloeil_INCLUDE_DIR
)

