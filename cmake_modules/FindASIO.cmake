# - Try to find the stand alone version of ASIO (also included in boost).
#
# Usage of this module as follows:
#
#     find_package(ASIO)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  ASIO_ROOT_DIR  Set this variable to the root installation of
#                    ASIO if the module has problems finding
#                    the proper installation path.
#
# Variables defined by this module:
#
#  ASIO_FOUND                    System has ASIO headers
#  ASIO_INCLUDE_DIR        The location of ASIO headers

find_path(ASIO_ROOT_DIR
        NAMES include/asio.hpp
        )

find_path(ASIO_INCLUDE_DIR
        NAMES asio.hpp
        HINTS ${ASIO_ROOT_DIR}/include
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASIO DEFAULT_MSG
        ASIO_INCLUDE_DIR
        )

mark_as_advanced(
        ASIO_ROOT_DIR
        ASIO_INCLUDE_DIR
)
