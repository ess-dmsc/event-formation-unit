find_path(FakeIt_ROOT_DIR
  NAMES include/fakeit.hpp
)

find_path(FakeIt_INCLUDE_DIR
  NAMES fakeit.hpp
  HINTS ${FakeIt_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FakeIt FOUND_VAR FakeIt_FOUND REQUIRED_VARS
  FakeIt_ROOT_DIR
  FakeIt_INCLUDE_DIR
)

mark_as_advanced(
  FakeIt_ROOT_DIR
  FakeIt_INCLUDE_DIR
)

