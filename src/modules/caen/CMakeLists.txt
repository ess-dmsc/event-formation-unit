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
  geometry/BifrostGeometry.h
  geometry/LokiGeometry.h
  geometry/MiraclesGeometry.h
  readout/DataParser.h
  )
set(caen_common_src
  CaenBase.cpp
  CaenInstrument.cpp
  geometry/Config.cpp
  geometry/Calibration.cpp
  geometry/BifrostGeometry.cpp
  geometry/LokiGeometry.cpp
  geometry/MiraclesGeometry.cpp
  readout/DataParser.cpp
  )

set(caen_INC ${caen_common_inc})
set(caen_SRC
  ${caen_common_src}
  ${ESS_SOURCE_DIR}/efu/main.cpp
  )
set(caen_LIB efu_reduction efu_essreadout)
create_executable(caen)

add_dependencies(efu_reduction efu_essreadout)
target_compile_definitions(caen PUBLIC EFU_CAEN)


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
