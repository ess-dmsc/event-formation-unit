#=============================================================================
# C++ standard
#=============================================================================
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 11)
  set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(EXTRA_CXX_FLAGS "-Werror -Wall -Wpedantic -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -O3 -DRELEASE -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} "-ggdb -fno-omit-frame-pointer")

add_definitions("-D__FAVOR_BSD") #Not working correctly?

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  message(STATUS "Detected MacOSX")
  add_definitions("-DSYSTEM_NAME_DARWIN")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(STATUS "Detected Linux")
  add_definitions("-DSYSTEM_NAME_LINUX")
  find_library(DL_LIB dl REQUIRED)
else()
  message(FATAL_ERROR "Unknown system")
endif()

#=============================================================================
# GCC ONLY
#=============================================================================
if(${CMAKE_COMPILER_IS_GNUCXX})
  set(CMAKE_AR "gcc-ar")
  set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_CXX_ARCHIVE_FINISH true)

  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
    message(STATUS "*** GCC>7 Compiler is too awesome. The following warnings are disabled:")
    message(STATUS "       no-implicit-fallthrough")
    message(STATUS "       no-aligned-new")
#    add_linker_flags(${module_name} "--no-as-needed")
    set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wno-implicit-fallthrough -Wno-aligned-new")
  endif()
endif()

#message(STATUS EXTRA_CXX_FLAGS = ${EXTRA_CXX_FLAGS})

add_definitions(${EXTRA_CXX_FLAGS})
