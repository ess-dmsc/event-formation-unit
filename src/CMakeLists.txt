include_directories(.)
include_directories(${VERSION_INCLUDE_DIR})

#=============================================================================
# Setup helpful paths to avoid relative includes in source files
#=============================================================================
set(ESS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(ESS_MODULE_DIR ${ESS_SOURCE_DIR}/modules/)

#=============================================================================
# Required external libraries
#=============================================================================

find_package(Threads REQUIRED)
find_package(LibRDKafka REQUIRED)
find_package(fmt REQUIRED)
find_package(GraylogLogger REQUIRED)
find_package(h5cpp REQUIRED)
find_package(PCAP REQUIRED)
find_package(asio REQUIRED)
find_package(jsonformoderncpp REQUIRED)

set(EFU_COMMON_LIBS
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  ${LibRDKafka_LIBRARIES}
  ${LibRDKafka_C_LIBRARIES}
  ${HDF5_C_LIBRARIES}
  ${Boost_LIBRARIES}
  GraylogLogger::graylog_logger_static
  fmt::fmt
  h5cpp::h5cpp
  )

set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} PARENT_SCOPE)

#=============================================================================
# Include subdirs, do not change order
#=============================================================================
add_subdirectory(common)
add_subdirectory(modules)
add_subdirectory(efu)
add_subdirectory(udpgenpcap)

get_filename_component(MB_CFG_FILE "${ESS_MODULE_DIR}/multiblade/configs/MB18Estia.json" ABSOLUTE)
get_filename_component(MG_CFG_FILE "${ESS_MODULE_DIR}/multigrid/configs/ILL_mappings.json" ABSOLUTE)
get_filename_component(GDGEM_CFG_FILE "${ESS_MODULE_DIR}/gdgem/configs/vmm3.json" ABSOLUTE)
get_filename_component(JALOUSIE_CFG_FILE "${ESS_MODULE_DIR}/jalousie/configs/v20_mappings.json" ABSOLUTE)
get_filename_component(JALOUSIE_CSV_FILE "${ESS_MODULE_DIR}/jalousie/configs/sumo_voxel_map_20190711.csv" ABSOLUTE)
get_filename_component(LOKI_CFG_FILE "${ESS_MODULE_DIR}/loki/configs/STFCTest.json" ABSOLUTE)

add_custom_target(copycfgs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/configs
    COMMAND ${CMAKE_COMMAND} -E copy ${MG_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${MB_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMENT "Copying ${MB_CFG_FILE} to ${PROJECT_BINARY_DIR}/configs"
    COMMAND ${CMAKE_COMMAND} -E copy ${GDGEM_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${JALOUSIE_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${JALOUSIE_CSV_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${LOKI_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMENT "Copying json configuration files to ${PROJECT_BINARY_DIR}/configs"
)

add_custom_target(runefu
    COMMAND efu "-d" "../modules/mgmesytec" "-f" "../configs/ILL_mappings.json" "-s" "1" "--dumptofile" "deleteme_" "--nohwcheck"
    COMMAND efu "-d" "../modules/sonde" "-s" "1" "--dumptofile" "deleteme_" "--nohwcheck"
    COMMAND efu "-d" "../modules/mbcaen" "-f" "../configs/MB18Estia.json"  "-s" "1" "--dumptofile" "deleteme_" "--nohwcheck"
    COMMAND efu "-d" "../modules/mbcaen" "-f" "../configs/MB18Estia.json"  "-s" "1" "--udder" "--nohwcheck"
    COMMAND efu "-d" "AdcReadout" "-s" "1" "--nohwcheck"
    COMMAND efu "-d" "../modules/loki" "-f" "../configs/STFCTest.json" "-s" "1" "--nohwcheck" "--dumptofile" "deleteme_"
    COMMAND efu "-d" "../modules/loki" "-f" "../configs/STFCTest.json" "-s" "1" "--nohwcheck" "--udder"
    COMMAND efu "-d" "../modules/gdgem" "-f" "../configs/vmm3.json" "-s" "2" "--nohwcheck"
    COMMAND efu "-d" "../modules/perfgen" "-s" "1" "--nohwcheck"
    COMMAND efu "-d" "../modules/jalousie" "-f" "../configs/v20_mappings.json" "-s" "2" "--nohwcheck"
    DEPENDS copycfgs efu mgmesytec gdgem sonde mbcaen AdcReadout
    )
