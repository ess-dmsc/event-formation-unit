#=============================================================================
# C++ standard
#=============================================================================
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 11)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(EXTRA_CXX_FLAGS "-Werror -Wall -Wpedantic -Wextra -Wno-deprecated")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -O3 -DRELEASE -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")
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
endif()

#message(STATUS EXTRA_CXX_FLAGS = ${EXTRA_CXX_FLAGS})

add_definitions(${EXTRA_CXX_FLAGS})
