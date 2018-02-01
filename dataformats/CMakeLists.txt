cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

#=============================================================================
# Augment CMake with our custom scripts
#=============================================================================
set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/../cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR}/modules)

#=============================================================================
# In case conan has generated info, use it now
#=============================================================================
if(EXISTS ${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  message(STATUS "Using existing conanbuildinfo.cmake file")
  include(${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup(KEEP_RPATHS)
endif()

include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)

add_subdirectory(multigrid)
