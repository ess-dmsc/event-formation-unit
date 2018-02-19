include_directories(. ..)
include(Utilities.cmake)

#=============================================================================
# Optionally dump raw data to file
#=============================================================================
option(DUMPTOFILE "Dump raw readout data to file " OFF)
if(${DUMPTOFILE})
  message(STATUS "Dumping of raw readout data to file enabled.")
  add_definitions("-DDUMPTOFILE")
else()
  message(STATUS "Dumping of raw readout data to file disabled (-DUMPTOFILE=NO)")
endif()

#=============================================================================
# Required external libraries
#=============================================================================
set(EFU_COMMON_LIBS ${CMAKE_DL_LIBS})

find_package(Threads REQUIRED)
set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  )

find_package(LibRDKafka REQUIRED)
set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS}
  ${LibRDKafka_LIBRARIES}
  ${LibRDKafka_C_LIBRARIES}
  )

#=============================================================================
# Optional external libraries
#=============================================================================
find_package(GraylogLogger)
if(GraylogLogger_FOUND)
  set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} ${GraylogLogger_LIBRARIES})
  add_definitions("-DGRAYLOG")
else()
  message(WARNING "GraylogLogger not found.")
endif()

find_package(h5cpp)
if(h5cpp_FOUND)
  set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} h5cpp ${HDF5_C_LIBRARIES} ${Boost_LIBRARIES})
else()
  message(WARNING "h5cpp not found.")
endif()


#=============================================================================
# Dependencies common to EFU and plugins
#=============================================================================

add_subdirectory(common)

add_subdirectory(readout)

include(UnitTests.cmake) # This has only libs unit tests now ???

#=============================================================================
# Event Formation Unit
#=============================================================================
add_subdirectory(efu)

#=============================================================================
# Plugins
#=============================================================================
add_subdirectory(gdgem)
add_subdirectory(multiblade)
add_subdirectory(multigrid)
add_subdirectory(sonde)
add_subdirectory(udp)
add_subdirectory(adc_readout)


#=============================================================================
#  These commands must be at the end of this file
#=============================================================================

# Should unit_test_targets depend on default targets ???

add_custom_target(runtest
  COMMAND ${CMAKE_CTEST_COMMAND} -V -R regular_*
  DEPENDS ${unit_test_targets})
add_custom_target(valgrind
  COMMAND ${CMAKE_CTEST_COMMAND} -R memcheck_*
  DEPENDS ${unit_test_targets})
add_custom_target(benchmark
  DEPENDS ${benchmark_targets})

add_custom_target(runefu
  COMMAND efu "-d" "../lib/mgmesytec" "-s" "1"
  DEPENDS efu mgmesytec
  )

