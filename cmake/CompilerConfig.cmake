# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 11)
  set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(EXTRA_CXX_FLAGS "-Werror -Wall -Wpedantic -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -flto -O3 -DRELEASE -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} "-ggdb -fno-omit-frame-pointer")

add_definitions("-D__FAVOR_BSD") #Not working correctly

if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
    message(STATUS "*** GCC>7 Compiler is too awesome. The following warnings are disabled:")
    message(STATUS "       no-implicit-fallthrough")
    message(STATUS "       no-aligned-new")
    set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-Wno-implicit-fallthrough -Wno-aligned-new")
endif ()

option(USE_OLD_ABI "Sets _GLIBCXX_USE_CXX11_ABI=0 for CentOS builds" ON)
if (${USE_OLD_ABI})
    message(WARNING "*** Setting _GLIBCXX_USE_CXX11_ABI=0 consider changing this when gcc on CentOS is updated")
    set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-D_GLIBCXX_USE_CXX11_ABI=0")
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    message(STATUS "Detected MacOSX")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Detected Linux")
    find_library(DL_LIB dl REQUIRED)
else ()
    message(FATAL_ERROR "Unknown system")
endif ()

if (${CMAKE_COMPILER_IS_GNUCXX})
    set(CMAKE_AR "gcc-ar")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_FINISH true)
endif ()