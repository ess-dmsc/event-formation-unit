# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# caen detector module and base test
#=============================================================================
set(caen_common_inc
  CaenBase.h
  CaenInstrument.h
  geometry/Config.h
  geometry/Calibration.h
  geometry/Geometry.h
  ${ESS_MODULE_DIR}/bifrost/geometry/BifrostGeometry.h
  ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.h
  ${ESS_MODULE_DIR}/miracles/geometry/MiraclesGeometry.h
  ${ESS_MODULE_DIR}/cspec/geometry/CspecGeometry.h
  readout/DataParser.h
  )
set(caen_common_src
  CaenBase.cpp
  CaenInstrument.cpp
  geometry/Config.cpp
  geometry/Calibration.cpp
  ${ESS_MODULE_DIR}/bifrost/geometry/BifrostGeometry.cpp
  ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.cpp
  ${ESS_MODULE_DIR}/miracles/geometry/MiraclesGeometry.cpp
  ${ESS_MODULE_DIR}/cspec/geometry/CspecGeometry.cpp
  readout/DataParser.cpp
  )

add_library(caen_common STATIC
  ${caen_common_src}
  ${caen_common_inc}
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

set(CaenBaseTest_INC
  ${caen_common_inc}
)
set(CaenBaseTest_SRC
  ${caen_common_src}
  test/CaenBaseTest.cpp
)
create_test_executable(CaenBaseTest)
