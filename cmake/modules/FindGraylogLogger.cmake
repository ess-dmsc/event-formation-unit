find_path(GraylogLogger_ROOT_DIR
        NAMES include/graylog_logger/Log.hpp
        PATHS /usr/local /opt/local/
        )

find_library(GraylogLogger_LIBRARIES
        NAMES graylog_logger
        HINTS ${GraylogLogger_ROOT_DIR}/lib
        )

find_library(GraylogLogger_STATIC_LIBRARIES
        NAMES graylog_logger_static
        HINTS ${GraylogLogger_ROOT_DIR}/lib
        )

find_path(GraylogLogger_INCLUDE_DIR
        NAMES graylog_logger/Log.hpp
        HINTS ${GraylogLogger_ROOT_DIR}/include
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GraylogLogger FOUND_VAR GraylogLogger_FOUND REQUIRED_VARS
        GraylogLogger_LIBRARIES
        GraylogLogger_STATIC_LIBRARIES
        GraylogLogger_INCLUDE_DIR
        )

mark_as_advanced(
        GraylogLogger_ROOT_DIR
        GraylogLogger_LIBRARIES
        GraylogLogger_STATIC_LIBRARIES
        GraylogLogger_INCLUDE_DIR
)
