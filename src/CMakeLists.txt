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
find_package(RdKafka REQUIRED)
find_package(fmt REQUIRED)
find_package(GraylogLogger REQUIRED)
find_package(h5cpp REQUIRED)
find_package(PCAP REQUIRED)
find_package(asio REQUIRED)
find_package(nlohmann_json REQUIRED)

set(EFU_COMMON_LIBS
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  RdKafka::rdkafka
  RdKafka::rdkafka++
  ${HDF5_C_LIBRARIES}
  ${Boost_LIBRARIES}
  GraylogLogger::graylog_logger_static
  fmt::fmt
  h5cpp::h5cpp
  nlohmann_json::nlohmann_json
  )

set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} PARENT_SCOPE)


#=============================================================================
# Include subdirs, do not change order
#=============================================================================
add_subdirectory(common)
add_subdirectory(efu)
add_subdirectory(modules)
add_subdirectory(generators)

get_filename_component(DREAM_CFG_FILE "${ESS_MODULE_DIR}/dream/configs/DreamEndcap.json" ABSOLUTE)
get_filename_component(FREIA_CFG_FILE "${ESS_MODULE_DIR}/freia/configs/freia.json" ABSOLUTE)
get_filename_component(CSPEC_CFG_FILE "${ESS_MODULE_DIR}/cspec/configs/cspec.json" ABSOLUTE)
get_filename_component(TTLMON_CFG_FILE "${ESS_MODULE_DIR}/ttlmonitor/configs/freiamon.json" ABSOLUTE)
get_filename_component(NMX_CFG_FILE "${ESS_MODULE_DIR}/nmx/configs/nmx.json" ABSOLUTE)


add_custom_target(copycfgs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/configs
    COMMAND ${CMAKE_COMMAND} -E copy ${DREAM_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${FREIA_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CSPEC_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${TTLMON_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${NMX_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMENT "Copying json configuration files to ${PROJECT_BINARY_DIR}/configs"
)

add_custom_target(runefu
    COMMAND caen "-f" "../configs/STFCTestII.json" "-s" "1" "--nohwcheck" "--dumptofile" "deleteme_"
    COMMAND perfgen "-s" "1" "--nohwcheck"
    COMMAND dream "-f" "../configs/DreamEndcap.json" "-s" "2" "--nohwcheck"
    COMMAND freia "-f" "../configs/freia.json" "-s" "2" "--nohwcheck" "-l" "5"
    COMMAND cspec "-f" "../configs/cspec.json" "-s" "2" "--nohwcheck"
    COMMAND ttlmonitor "-f" "../configs/freiamon.json" "-s" "2" "--nohwcheck" "--dumptofile" "deleteme_ttlmon_" "-t" "freia_beam_monitor"
    COMMAND nmx "-f" "../configs/nmx.json" "-s" "2"  "--nohwcheck"
    DEPENDS copycfgs efu dream caen freia cspec ttlmonitor nmx
    )
