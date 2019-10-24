include_directories(.)
include_directories(${VERSION_INCLUDE_DIR})

#=============================================================================
# Required external libraries
#=============================================================================

find_package(Threads REQUIRED)
find_package(LibRDKafka REQUIRED)
find_package(fmt REQUIRED)
find_package(GraylogLogger REQUIRED)
find_package(h5cpp REQUIRED)
find_package(PCAP REQUIRED)

set(EFU_COMMON_LIBS
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  ${LibRDKafka_LIBRARIES}
  ${LibRDKafka_C_LIBRARIES}
  ${HDF5_C_LIBRARIES}
  ${Boost_LIBRARIES}
  GraylogLogger::graylog_logger_static
  fmt::fmt
  h5cpp
  )

set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} PARENT_SCOPE)

#=============================================================================
# Dependencies common to EFU and plugins
#=============================================================================

add_subdirectory(common)

add_subdirectory(readout)

add_subdirectory(generators)

#=============================================================================
# Plugins
#=============================================================================
add_subdirectory(gdgem)
add_subdirectory(multiblade)
add_subdirectory(multigrid)
add_subdirectory(sonde)
add_subdirectory(jalousie)
add_subdirectory(udp)
add_subdirectory(adc_readout)
add_subdirectory(perfgen)
add_subdirectory(tools)

##=============================================================================
## Event Formation Unit
##=============================================================================
# Add it last.
add_subdirectory(efu)

get_filename_component(MB_CFG_FILE "multiblade/configs/MB18Estia.json" ABSOLUTE)
get_filename_component(MG_CFG_FILE "multigrid/configs/ILL_mappings.json" ABSOLUTE)
get_filename_component(GDGEM_CFG_FILE "gdgem/configs/vmm3.json" ABSOLUTE)
add_custom_target(copycfgs
COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/configs
COMMAND ${CMAKE_COMMAND} -E copy ${MG_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
COMMAND ${CMAKE_COMMAND} -E copy ${MB_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
COMMENT "Copying ${MB_CFG_FILE} to ${PROJECT_BINARY_DIR}/configs"
COMMAND ${CMAKE_COMMAND} -E copy ${GDGEM_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
COMMENT "Copying json configuration files to ${PROJECT_BINARY_DIR}/configs"
)

add_custom_target(runefu
    COMMAND efu "-d" "../modules/mgmesytec" "-f" "../configs/ILL_mappings.json" "-s" "1" "--dumptofile" "deleteme_" "--nohwcheck"
    COMMAND efu "-d" "../modules/sonde" "-s" "1" "--dumptofile" "deleteme_" "--nohwcheck"
    COMMAND efu "-d" "../modules/mbcaen" "-f" "../configs/MB18Estia.json"  "-s" "1" "--dumptofile" "deleteme_" "--nohwcheck"
    COMMAND efu "-d" "AdcReadout" "-s" "1" "--nohwcheck"
    COMMAND efu "-d" "../modules/udp" "-s" "1" "--nohwcheck"
    COMMAND efu "-d" "../modules/gdgem" "-f" "../configs/vmm3.json" "-s" "2" "--nohwcheck"
    DEPENDS copycfgs efu mgmesytec gdgem sonde mbcaen udp AdcReadout
    )
