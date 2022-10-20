
add_subdirectory(generators)
add_subdirectory(test)

include_directories(.)

#=============================================================================
# caen detector module and base test
#=============================================================================
set(caen_common_inc
  Counters.h
  CaenBase.h
  CaenInstrument.h
  geometry/PanelGeometry.h
  geometry/Config.h
  geometry/Calibration.h
  geometry/LokiGeometry.h
  readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.h
  )
set(caen_common_src
  CaenBase.cpp
  CaenInstrument.cpp
  geometry/Config.cpp
  geometry/Calibration.cpp
  geometry/LokiGeometry.cpp
  readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.cpp
  )
set(caen_INC ${caen_common_inc})
set(caen_SRC Caen.cpp ${caen_common_src})
create_module(caen)

#
set(CaenBaseTest_INC
  ${caen_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(CaenBaseTest_SRC
  test/CaenBaseTest.cpp
  ${caen_common_src}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(CaenBaseTest)

##
## CaenInstrumentTest Module integration test
##
set(CaenInstrumentTest_INC
  ${caen_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(CaenInstrumentTest_SRC
  ${caen_common_src}
  test/CaenInstrumentTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
)
create_test_executable(CaenInstrumentTest)
