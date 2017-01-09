cmake_minimum_required (VERSION 3.5)

set (CMAKE_CXX_STANDARD 11)

project (main)

set(CMAKE_CXX_FLAGS_DEBUG   " -O3 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs")
set(CMAKE_CXX_FLAGS_RELEASE " -O3 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs")

include_directories(${main_SOURCE_DIR})

find_library(DL_LIB dl)

add_executable(main main.cpp Pipeline.cpp)
target_link_libraries(main ${DL_LIB})


add_library(nmx SHARED liba.cpp)
SET_TARGET_PROPERTIES(nmx PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(nmx PROPERTIES SUFFIX ".inst")

add_library(beer SHARED libb.cpp)
SET_TARGET_PROPERTIES(beer PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(beer PROPERTIES SUFFIX ".inst")
