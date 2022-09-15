#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CAEN_FULL "${ESS_MODULE_DIR}/caen/configs/Caen.json" ABSOLUTE)

##
## Pure unit tests
##

##
## Analog geometry - amplitude calculations
##
set(TubeAmpsTest_INC ../geometry/TubeAmps.h)
set(TubeAmpsTest_SRC TubeAmpsTest.cpp)
create_test_executable(TubeAmpsTest)

##
## Panel geometry
##
set(CaenPanelGeometryTest_INC
  ../geometry/TubeAmps.h
  ../geometry/PanelGeometry.h)
set(CaenPanelGeometryTest_SRC PanelGeometryTest.cpp)
create_test_executable(CaenPanelGeometryTest)

##
## Validate LoKI geometry between EFU and Nexus Constructor
##
set(CaenFullGeometryTest_INC
  CaenFullGeometryTestData.h
  ../geometry/TubeAmps.h
  ../geometry/PanelGeometry.h)
set(CaenFullGeometryTest_SRC CaenFullGeometryTest.cpp)
create_test_executable(CaenFullGeometryTest)


set(CaenCalibrationTest_INC
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ../geometry/Config.h
  ../geometry/Calibration.h)
set(CaenCalibrationTest_SRC
  ../geometry/Config.cpp
  ../geometry/Calibration.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  CalibrationTest.cpp)
create_test_executable(CaenCalibrationTest)


set(CaenConfigTest_INC
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ../geometry/Config.h)
set(CaenConfigTest_SRC
  ../geometry/Config.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ConfigTest.cpp)
create_test_executable(CaenConfigTest)
target_compile_definitions(CaenConfigTest PRIVATE CAEN_FULL="${CAEN_FULL}")

##
## readout/
##
set(CaenDataParserTest_INC
  ../readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h)
set(CaenDataParserTest_SRC
  ../readout/DataParser.cpp
  DataParserTest.cpp)
create_test_executable(CaenDataParserTest)


##
## Module Integration tests below
##


##
## Module integration test for both generic readout and LoKI
##
set(CombinedParserTest_INC
  ../readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ../generators/ReadoutGenerator.h
  )
set(CombinedParserTest_SRC
  CombinedParserTest.cpp
  ../generators/ReadoutGenerator.cpp
  ../readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  )
create_test_executable(CombinedParserTest)
