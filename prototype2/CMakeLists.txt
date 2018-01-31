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

enable_coverage(efu2)

#
#  These commands must be at the end of this file
#
add_custom_target(runtest
  COMMAND ${CMAKE_CTEST_COMMAND} -V -R regular_*
  DEPENDS ${unit_test_targets})
add_custom_target(valgrind
  COMMAND ${CMAKE_CTEST_COMMAND} -R memcheck_*
  DEPENDS ${unit_test_targets})
add_custom_target(benchmark
  DEPENDS ${benchmark_targets})

