#To set a H5CC root directory, set H5CC_ROOT_DIR

find_path(H5CC_ROOT_DIR
    NAMES include/h5cc/H5CC_File.h
    HINTS /usr/local/
)

find_path(H5CC_INCLUDE_DIR
    NAMES H5CC_File.h
    HINTS ${H5CC_ROOT_DIR}/include/h5cc
)

find_library(H5CC_LIBRARY
    NAMES h5cc
    HINTS ${H5CC_ROOT_DIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(H5CC FOUND_VAR H5CC_FOUND REQUIRED_VARS
    H5CC_LIBRARY
    H5CC_INCLUDE_DIR
)

mark_as_advanced(
    H5CC_ROOT_DIR
    H5CC_INCLUDE_DIR
    H5CC_LIBRARY
)