# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

get_filename_component(LOKI_CONFIG "${ESS_MODULE_DIR}/loki/configs/LokiFull.json" ABSOLUTE)
get_filename_component(LOKI_CALIB "${ESS_MODULE_DIR}/loki/configs/lokinullcalib.json" ABSOLUTE)
get_filename_component(BIFROST_CONFIG "${ESS_MODULE_DIR}/bifrost/configs/bifrost.json" ABSOLUTE)
get_filename_component(BIFROST_CALIB "${ESS_MODULE_DIR}/bifrost/configs/bifrostnullcalib.json" ABSOLUTE)
get_filename_component(TBL3HE_CONFIG "${ESS_MODULE_DIR}/tbl3he/configs/tbl3he.json" ABSOLUTE)
get_filename_component(TBL3HE_CALIB "${ESS_MODULE_DIR}/tbl3he/configs/tbl3henullcalib.json" ABSOLUTE)
get_filename_component(MIRACLES_CONFIG "${ESS_MODULE_DIR}/miracles/configs/miracles.json" ABSOLUTE)
get_filename_component(MIRACLES_CALIB "${ESS_MODULE_DIR}/miracles/configs/miraclesnullcalib.json" ABSOLUTE)
get_filename_component(CSPEC_CONFIG "${ESS_MODULE_DIR}/cspec/configs/cspec.json" ABSOLUTE)
get_filename_component(CSPEC_CALIB "${ESS_MODULE_DIR}/cspec/configs/cspecnullcalib.json" ABSOLUTE)

#=============================================================================
# caen detector module and base test
#=============================================================================
set(caen_common_inc
  CaenBase.h
  CaenCounters.h
  CaenInstrument.h
  geometry/Config.h
  geometry/CDCalibration.h
  geometry/Geometry.h
  geometry/Interval.h
  ${ESS_MODULE_DIR}/bifrost/geometry/BifrostGeometry.h
  ${ESS_MODULE_DIR}/cspec/geometry/CspecGeometry.h
  ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
  ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.h
  ${ESS_MODULE_DIR}/miracles/geometry/MiraclesGeometry.h
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.h
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeGeometry.h
  readout/DataParser.h
  )
set(caen_common_src
  CaenBase.cpp
  CaenInstrument.cpp
  geometry/Config.cpp
  geometry/CDCalibration.cpp
  geometry/Interval.cpp
  ${ESS_MODULE_DIR}/bifrost/geometry/BifrostGeometry.cpp
  ${ESS_MODULE_DIR}/cspec/geometry/CspecGeometry.cpp
  ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
  ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.cpp
  ${ESS_MODULE_DIR}/miracles/geometry/MiraclesGeometry.cpp
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.cpp
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeGeometry.cpp
  readout/DataParser.cpp
  )

add_library(caen_common STATIC
  ${caen_common_src}
  ${caen_common_inc}
  )

target_link_libraries(caen_common
  PRIVATE
    ${EFU_COMMON_LIBS}
)

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(caen_common "-Wl,--no-as-needed")
endif()

##
## CaeniInstrumentTest Module integration test
##
set(CaenInstrumentTest_INC
  ${caen_common_inc}
  )
set(CaenInstrumentTest_SRC
  ${caen_common_src}
  test/CaenInstrumentTest.cpp
)
create_test_executable(CaenInstrumentTest)
target_compile_definitions(CaenInstrumentTest PRIVATE LOKI_CONFIG="${LOKI_CONFIG}")
target_compile_definitions(CaenInstrumentTest PRIVATE LOKI_CALIB="${LOKI_CALIB}")
target_compile_definitions(CaenInstrumentTest PRIVATE BIFROST_CONFIG="${BIFROST_CONFIG}")
target_compile_definitions(CaenInstrumentTest PRIVATE BIFROST_CALIB="${BIFROST_CALIB}")
target_compile_definitions(CaenInstrumentTest PRIVATE TBL3HE_CONFIG="${TBL3HE_CONFIG}")
target_compile_definitions(CaenInstrumentTest PRIVATE TBL3HE_CALIB="${TBL3HE_CALIB}")
target_compile_definitions(CaenInstrumentTest PRIVATE CSPEC_CONFIG="${CSPEC_CONFIG}")
target_compile_definitions(CaenInstrumentTest PRIVATE CSPEC_CALIB="${CSPEC_CALIB}")

set(CaenBaseTest_INC
  ${caen_common_inc}
)
set(CaenBaseTest_SRC
  ${caen_common_src}
  test/CaenBaseTest.cpp
)
create_test_executable(CaenBaseTest)
target_compile_definitions(CaenBaseTest PRIVATE LOKI_CONFIG="${LOKI_CONFIG}")
target_compile_definitions(CaenBaseTest PRIVATE LOKI_CALIB="${LOKI_CALIB}")
target_compile_definitions(CaenBaseTest PRIVATE BIFROST_CONFIG="${BIFROST_CONFIG}")
target_compile_definitions(CaenBaseTest PRIVATE BIFROST_CALIB="${BIFROST_CALIB}")
target_compile_definitions(CaenBaseTest PRIVATE MIRACLES_CONFIG="${MIRACLES_CONFIG}")
target_compile_definitions(CaenBaseTest PRIVATE MIRACLES_CALIB="${MIRACLES_CALIB}")
target_compile_definitions(CaenBaseTest PRIVATE CSPEC_CONFIG="${CSPEC_CONFIG}")
target_compile_definitions(CaenBaseTest PRIVATE CSPEC_CALIB="${CSPEC_CALIB}")
