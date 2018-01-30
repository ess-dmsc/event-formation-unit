# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 11)
  set(CMAKE_CXX_STANDARD 11)
endif()

set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS}
    "-Werror -Wall -Wpedantic -Wextra -Wno-missing-include-dirs")

set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -flto -O3 -DRELEASE -DNDEBUG")

if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
    # Hackity-hack...
    message(STATUS "*** GCC>7 Compiler is too awesome. The following warnings are disabled:")
    message(STATUS "       no-implicit-fallthrough")
    message(STATUS "       no-aligned-new")
    set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} "-Wno-implicit-fallthrough -Wno-aligned-new")
endif ()
