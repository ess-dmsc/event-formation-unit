cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("event-formation-unit")
set(event-formation-unit_VERSION_MAJOR 1)
set(event-formation-unit_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

find_package(GTest)

add_subdirectory(libs)
add_subdirectory(prototype2)
