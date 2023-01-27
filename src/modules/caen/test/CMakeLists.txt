#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CAEN_FULL "${ESS_MODULE_DIR}/loki/configs/Loki.json" ABSOLUTE)

##
## Pure unit tests
##


set(CaenCalibrationTest_INC
  ${ESS_MODULE_DIR}/caen/geometry/Config.h
  ${ESS_MODULE_DIR}/caen/geometry/Calibration.h)
set(CaenCalibrationTest_SRC
  ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
  ${ESS_MODULE_DIR}/caen/geometry/Calibration.cpp
  CalibrationTest.cpp)
create_test_executable(CaenCalibrationTest)


set(CaenConfigTest_INC
  ${ESS_MODULE_DIR}/caen/geometry/Config.h)
set(CaenConfigTest_SRC
  ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
  ConfigTest.cpp)
create_test_executable(CaenConfigTest)
target_compile_definitions(CaenConfigTest PRIVATE CAEN_FULL="${CAEN_FULL}")

##
## readout/
##
set(CaenDataParserTest_INC
  ${ESS_MODULE_DIR}/caen/readout/DataParser.h
  ${ESS_COMMON_DIR}/readout/ess/Parser.h)
set(CaenDataParserTest_SRC
  ${ESS_MODULE_DIR}/caen/readout/DataParser.cpp
  DataParserTest.cpp)
create_test_executable(CaenDataParserTest)


##
## Module Integration tests below
##


##
## Module integration test for both generic readout and LoKI
##
set(CombinedParserTest_INC
  ${ESS_MODULE_DIR}/caen/readout/DataParser.h
  ${ESS_MODULE_DIR}/loki/generators/LokiReadoutGenerator.h
  ${ESS_COMMON_DIR}/readout/ess/Parser.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  )
set(CombinedParserTest_SRC
  CombinedParserTest.cpp
  ${ESS_MODULE_DIR}/loki/generators/LokiReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/caen/readout/DataParser.cpp
  ${ESS_COMMON_DIR}/readout/ess/Parser.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  )
create_test_executable(CombinedParserTest)
