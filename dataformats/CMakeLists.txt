cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/../cmake_modules)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})

include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)

#=============================================================================
# setting up conan for use from within cmake
#=============================================================================
if (EXISTS ${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  message(STATUS "Using existing conanbuildinfo.cmake file")
  include(${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup(KEEP_RPATHS)
endif ()

add_subdirectory(multigrid)
