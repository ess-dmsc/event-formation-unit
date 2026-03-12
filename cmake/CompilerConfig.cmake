#=============================================================================
# C++ standard
#=============================================================================
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 11)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Common flags: supported by both GCC 11+ and Clang
set(EXTRA_CXX_FLAGS
  "-Werror"
  "-Wall"
  "-Wpedantic"
  "-Wno-deprecated"
  "-Wdeprecated-copy"
  "-Wempty-body"
  "-Wexpansion-to-defined"
  "-Wignored-qualifiers"
  "-Wimplicit-fallthrough"
  "-Wmissing-field-initializers"
  "-Wredundant-move"
  "-Wshift-negative-value"
  "-Wsign-compare"
  "-Wstring-compare"
  "-Wtype-limits"
  "-Wuninitialized"
  "-Wunused-parameter"
  "-Wunused-but-set-parameter"
  "-Wtautological-compare"
)

# GCC-specific warnings (make production GCC stricter than Clang)
if(CMAKE_COMPILER_IS_GNUCXX)
  list(APPEND EXTRA_CXX_FLAGS
    "-Wcast-function-type"
    "-Wclobbered"
    "-Wmaybe-uninitialized"
    "-Wsized-deallocation"
  )
endif()

string(REPLACE ";" " " EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -O3 -g0 -DRELEASE -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")
#set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} "-ggdb -fno-omit-frame-pointer")

# Enable LTO and stripping for Release builds
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -s")
string(APPEND CMAKE_SHARED_LINKER_FLAGS_RELEASE " -s")
string(APPEND CMAKE_MODULE_LINKER_FLAGS_RELEASE " -s")

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
