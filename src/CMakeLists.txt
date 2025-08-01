include_directories(.)
include_directories(${VERSION_INCLUDE_DIR})

#=============================================================================
# Setup helpful paths to avoid relative includes in source files
#=============================================================================
set(ESS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(ESS_MODULE_DIR ${ESS_SOURCE_DIR}/modules/)
set(ESS_COMMON_DIR ${ESS_SOURCE_DIR}/common/)

#=============================================================================
# Required external libraries
#=============================================================================

find_package(Threads REQUIRED)
find_package(RdKafka REQUIRED)
find_package(fmt REQUIRED)
find_package(graylog-logger REQUIRED)
find_package(libpcap REQUIRED)
find_package(asio REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(CLI11 REQUIRED)
find_package(magic_enum REQUIRED)
find_package(streaming-data-types REQUIRED)
find_package(logical-geometry REQUIRED)
find_package(FakeIt REQUIRED)

set(EFU_COMMON_LIBS
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  RdKafka::rdkafka
  RdKafka::rdkafka++
  ${Boost_LIBRARIES}
  GraylogLogger::graylog_logger
  fmt::fmt
  nlohmann_json::nlohmann_json
  magic_enum::magic_enum
  streaming-data-types::streaming-data-types
  logical-geometry::logical-geometry
  CLI11::CLI11
  libpcap::libpcap
  fakeit::fakeit
  )

set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} PARENT_SCOPE)

#=============================================================================
# Identify the Linux distribution
#=============================================================================

# Add CentOS-specific configuration here
set(REDHAT_RELEASE_FILE "/etc/redhat-release")
set(UBUNTU_RELEASE_FILE "/etc/lsb-release")
set(ALMA_LINUX_RELEASE_FILE "/etc/alma-release")

if(EXISTS ${REDHAT_RELEASE_FILE})
  execute_process(COMMAND cat ${REDHAT_RELEASE_FILE}
          OUTPUT_VARIABLE LINUX_DISTRIBUTION
          OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(EXISTS ${UBUNTU_RELEASE_FILE})
  execute_process(COMMAND cat ${UBUNTU_RELEASE_FILE}
          OUTPUT_VARIABLE LINUX_DISTRIBUTION
          OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(EXISTS ${ALMA_LINUX_RELEASE_FILE})
  execute_process(COMMAND cat ${ALMA_LINUX_RELEASE_FILE}
          OUTPUT_VARIABLE LINUX_DISTRIBUTION
          OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

#=============================================================================
# Include subdirs, do not change order
#=============================================================================
add_subdirectory(common)
add_subdirectory(efu)
add_subdirectory(modules)
add_subdirectory(generators)

get_filename_component(DREAM_CFG_FILE "${ESS_MODULE_DIR}/dream/configs/DreamEndcap.json" ABSOLUTE)
get_filename_component(HEIMDAL_CFG_FILE "${ESS_MODULE_DIR}/dream/configs/HeimdalInst.json" ABSOLUTE)
get_filename_component(MAGIC_CFG_FILE "${ESS_MODULE_DIR}/dream/configs/MagicInst.json" ABSOLUTE)
get_filename_component(FREIA_CFG_FILE "${ESS_MODULE_DIR}/freia/configs/freia.json" ABSOLUTE)
get_filename_component(TREX_CFG_FILE "${ESS_MODULE_DIR}/trex/configs/trex.json" ABSOLUTE)
get_filename_component(CBM_CFG_FILE "${ESS_MODULE_DIR}/cbm/configs/cbmtest.json" ABSOLUTE)
get_filename_component(NMX_CFG_FILE "${ESS_MODULE_DIR}/nmx/configs/nmx.json" ABSOLUTE)
get_filename_component(LOKI_CFG_FILE "${ESS_MODULE_DIR}/loki/configs/LokiFull.json" ABSOLUTE)
get_filename_component(LOKI_CAL_FILE "${ESS_MODULE_DIR}/loki/configs/lokinullcalib.json" ABSOLUTE)
get_filename_component(BIFROST_CFG_FILE "${ESS_MODULE_DIR}/bifrost/configs/bifrost.json" ABSOLUTE)
get_filename_component(BIFROST_CAL_FILE "${ESS_MODULE_DIR}/bifrost/configs/bifrostnullcalib.json" ABSOLUTE)
get_filename_component(MIRACLES_CFG_FILE "${ESS_MODULE_DIR}/miracles/configs/miracles.json" ABSOLUTE)
get_filename_component(MIRACLES_CAL_FILE "${ESS_MODULE_DIR}/miracles/configs/miraclesnullcalib.json" ABSOLUTE)
get_filename_component(CSPEC_CFG_FILE "${ESS_MODULE_DIR}/cspec/configs/cspec.json" ABSOLUTE)
get_filename_component(CSPEC_CAL_FILE "${ESS_MODULE_DIR}/cspec/configs/cspecnullcalib.json" ABSOLUTE)
get_filename_component(ODIN_CFG_FILE "${ESS_MODULE_DIR}/timepix3/configs/timepix3.json" ABSOLUTE)
get_filename_component(TBL3HE_CFG_FILE "${ESS_MODULE_DIR}/tbl3he/configs/tbl3he.json" ABSOLUTE)
get_filename_component(TBL3HE_CAL_FILE "${ESS_MODULE_DIR}/tbl3he/configs/tbl3henullcalib.json" ABSOLUTE)


add_custom_target(copycfgs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/configs
    COMMAND ${CMAKE_COMMAND} -E copy ${DREAM_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${HEIMDAL_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${MAGIC_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${FREIA_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${TREX_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CBM_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${NMX_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${LOKI_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${LOKI_CAL_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${BIFROST_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${BIFROST_CAL_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${MIRACLES_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${MIRACLES_CAL_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CSPEC_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CSPEC_CAL_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${ODIN_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${TBL3HE_CFG_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMAND ${CMAKE_COMMAND} -E copy ${TBL3HE_CAL_FILE} ${PROJECT_BINARY_DIR}/configs/
    COMMENT "Copying json configuration files to ${PROJECT_BINARY_DIR}/configs"
)

add_custom_target(allefus DEPENDS dream loki bifrost miracles freia trex cbm nmx cspec timepix3)

add_custom_target(runefu
    COMMAND loki     "-f" "../configs/LokiFull.json" "--calibration" "../configs/lokinullcalib.json"     "-s" "1" "--nohwcheck"
    COMMAND bifrost  "-f" "../configs/bifrost.json"    "--calibration" "../configs/bifrostnullcalib.json"  "-s" "2" "--nohwcheck"
    COMMAND miracles "-f" "../configs/miracles.json"   "--calibration" "../configs/miraclesnullcalib.json" "-s" "2" "--nohwcheck"
    COMMAND cspec    "-f" "../configs/cspec.json"      "--calibration" "../configs/cspecnullcalib.json"    "-s" "2" "--nohwcheck"
    COMMAND perfgen  "-s" "1" "--nohwcheck"
    COMMAND dream    "-f" "../configs/DreamEndcap.json" "-s" "2" "--nohwcheck"
    COMMAND heimdal  "-f" "../configs/HeimdalInst.json" "-s" "2" "--nohwcheck"
    COMMAND magic    "-f" "../configs/MagicInst.json" "-s" "2" "--nohwcheck"
    COMMAND freia    "-f" "../configs/freia.json" "-s" "2" "--nohwcheck" "-l" "Info" "--log_file" "deleteme_graylog.log"
    COMMAND estia    "-f" "../configs/estia.json" "-s" "2" "--nohwcheck"
    COMMAND trex     "-f" "../configs/trex.json" "-s" "2" "--nohwcheck"
    COMMAND cbm      "-f" "../configs/cbmtest.json" "-s" "2" "--nohwcheck" "-t" "freia_beam_monitor"
    COMMAND nmx      "-f" "../configs/nmx.json" "-s" "2"  "--nohwcheck"
    COMMAND timepix3 "-f" "../configs/timepix3.json" "-s" "2" "--nohwcheck"
    COMMAND tbl3he   "-f" "../configs/tbl3he.json"    "--calibration" "../configs/tbl3henullcalib.json"  "-s" "2" "--nohwcheck"
    DEPENDS copycfgs allefus
    )
