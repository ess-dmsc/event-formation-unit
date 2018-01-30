cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

enable_testing()

include_directories(. ..)

#
# Dependencies common to EFU and plugins
#
add_subdirectory(common)

include(Utilities.cmake)
include(UnitTests.cmake)

#
# Plugins
#
add_subdirectory(gdgem)
add_subdirectory(multiblade)
add_subdirectory(multigrid)
add_subdirectory(sonde)
add_subdirectory(udp)


#
# the E F U
#
set(efu2_SRC
  common/EFUArgs.cpp
  common/StatPublisher.cpp
  efu/Loader.cpp
  efu/Launcher.cpp
  efu/main.cpp
  efu/ExitHandler.cpp
  efu/Server.cpp
  efu/Parser.cpp
  )
set(efu2_INC
  CLI/CLI11.hpp
  common/Trace.h
  common/StatPublisher.h
  common/NewStats.h
  common/EFUArgs.h
  efu/Launcher.h
  efu/Loader.h
  efu/Server.h
  efu/Parser.h
  efu/ExitHandler.h
  )
create_executable(efu2 "${CMAKE_DL_LIBS}")
