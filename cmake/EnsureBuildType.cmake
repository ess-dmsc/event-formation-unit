option(CMAKE_BUILD_TYPE "Build type; \"Release\" or \"Debug\"." Debug)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug)
else ()
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()
