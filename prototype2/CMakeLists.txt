include_directories(.)

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
find_package(GraylogLogger REQUIRED)
set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} ${GraylogLogger_STATIC_LIBRARIES})
add_definitions("-DGRAYLOG")

find_package(h5cpp REQUIRED)
set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} h5cpp ${HDF5_C_LIBRARIES} ${Boost_LIBRARIES})

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

add_custom_target(runefu
    COMMAND efu "-d" "../lib/mgmesytec" "-s" "1"
    DEPENDS efu mgmesytec
    )


set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} PARENT_SCOPE)
