find_path(ASIO_ROOT_DIR
  NAMES include/asio.hpp
  PATHS /usr/local /opt/local/
)

find_path(ASIO_INCLUDE_DIR
  NAMES asio.hpp
  HINTS ${ASIO_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASIO FOUND_VAR ASIO_FOUND REQUIRED_VARS
  ASIO_ROOT_DIR
  ASIO_INCLUDE_DIR
)

mark_as_advanced(
  ASIO_ROOT_DIR
  ASIO_INCLUDE_DIR
)
