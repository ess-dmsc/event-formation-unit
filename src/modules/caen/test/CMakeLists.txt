#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

##
## Pure unit tests
##


# New calibration scheme - validation tests
set(CaenCDCalibrationTest_INC
  ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
  ${ESS_MODULE_DIR}/caen/geometry/Interval.h
  )
set(CaenCDCalibrationTest_SRC
  ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
  ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
  CDCalibrationTest.cpp)
create_test_executable(CaenCDCalibrationTest)


# New calibration scheme - correction tests
set(CaenCDCalibrationIITest_INC
  ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
  ${ESS_MODULE_DIR}/caen/geometry/Interval.h
  )
set(CaenCDCalibrationIITest_SRC
  ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
  ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
  CDCalibrationIITest.cpp)
create_test_executable(CaenCDCalibrationIITest)


# Interval overlap checks
set(IntervalTest_INC
  ${ESS_MODULE_DIR}/caen/geometry/Interval.h)
set(IntervalTest_SRC
  ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
  ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
  IntervalTest.cpp)
create_test_executable(IntervalTest)


set(CaenConfigTest_INC
  ${ESS_MODULE_DIR}/caen/geometry/Config.h)
set(CaenConfigTest_SRC
  ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
  ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
  ConfigTest.cpp)
create_test_executable(CaenConfigTest)

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
