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
  ${ESS_SOURCE_DIR}/modules/bifrost/geometry/BifrostGeometry.h
  ${ESS_SOURCE_DIR}/modules/loki/geometry/LokiGeometry.h
  ${ESS_SOURCE_DIR}/modules/miracles/geometry/MiraclesGeometry.h
  readout/DataParser.h
  )
set(caen_common_src
  CaenBase.cpp
  CaenInstrument.cpp
  geometry/Config.cpp
  geometry/Calibration.cpp
  ${ESS_SOURCE_DIR}/modules/bifrost/geometry/BifrostGeometry.cpp
  ${ESS_SOURCE_DIR}/modules/loki/geometry/LokiGeometry.cpp
  ${ESS_SOURCE_DIR}/modules/miracles/geometry/MiraclesGeometry.cpp
  readout/DataParser.cpp
  )

add_library(caen_common STATIC
  ${caen_common_src}
  ${caen_common_inc}
  )


##
## CaeniInstrumentTest Module integration test
##
set(CaenInstrumentTest_INC
  ${caen_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(CaenInstrumentTest_SRC
  ${caen_common_src}
  test/CaenInstrumentTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Config.cpp

)
create_test_executable(CaenInstrumentTest)
