#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(LOKI_FULL "${ESS_MODULE_DIR}/loki/configs/Loki.json" ABSOLUTE)

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
set(LokiPanelGeometryTest_INC
  ../geometry/TubeAmps.h
  ../geometry/PanelGeometry.h)
set(LokiPanelGeometryTest_SRC PanelGeometryTest.cpp)
create_test_executable(LokiPanelGeometryTest)

##
## Validate LoKI geometry between EFU and Nexus Constructor
##
set(LokiFullGeometryTest_INC
  LokiFullGeometryTestData.h
  ../geometry/TubeAmps.h
  ../geometry/PanelGeometry.h)
set(LokiFullGeometryTest_SRC LokiFullGeometryTest.cpp)
create_test_executable(LokiFullGeometryTest)


set(LokiCalibrationTest_INC
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ../geometry/Config.h
  ../geometry/Calibration.h)
set(LokiCalibrationTest_SRC
  ../geometry/Config.cpp
  ../geometry/Calibration.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  CalibrationTest.cpp)
create_test_executable(LokiCalibrationTest)


set(LokiConfigTest_INC
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ../geometry/Config.h)
set(LokiConfigTest_SRC
  ../geometry/Config.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ConfigTest.cpp)
create_test_executable(LokiConfigTest)
target_compile_definitions(LokiConfigTest PRIVATE LOKI_FULL="${LOKI_FULL}")

##
## readout/
##
set(LokiDataParserTest_INC
  ../readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/readout/common/Parser.h)
set(LokiDataParserTest_SRC
  ../readout/DataParser.cpp
  DataParserTest.cpp)
create_test_executable(LokiDataParserTest)


##
## Module Integration tests below
##


##
## Module integration test for both generic readout and LoKI
##
set(CombinedParserTest_INC
  ../readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/readout/common/Parser.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(CombinedParserTest_SRC
  ${ESS_SOURCE_DIR}/common/readout/common/Parser.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  ../readout/DataParser.cpp
  CombinedParserTest.cpp
  ReadoutGenerator.cpp)
create_test_executable(CombinedParserTest)
