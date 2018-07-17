include_directories(.)

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

##=============================================================================
## Event Formation Unit
##=============================================================================
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
    COMMAND efu "-d" "../modules/mgmesytec" "-s" "1" "--dumptofile" "deleteme_" "-u" "1500"
    COMMAND efu "-d" "../modules/mgcncs2" "-s" "1" "--dumptofile" "deleteme_" "-u" "1500"
    COMMAND efu "-d" "../modules/sonde" "-s" "1" "--dumptofile" "deleteme_" "-u" "1500"
    COMMAND efu "-d" "../modules/mbcaen" "-s" "1" "--dumptofile" "deleteme_" "-u" "1500"
    COMMAND efu "-d" "../modules/AdcReadout" "-s" "1" "-u" "1500"
    COMMAND efu "-d" "../modules/udp" "-s" "1" "-u" "1500"
    COMMAND efu "-d" "../modules/gdgem" "-f" "nocfgfile" "-s" "2" "-u" "1500"
    DEPENDS efu mgmesytec mgcncs2 gdgem sonde mbcaen udp AdcReadout
    )

set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} PARENT_SCOPE)
