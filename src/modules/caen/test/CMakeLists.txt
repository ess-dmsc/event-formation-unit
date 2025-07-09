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
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.h
  ${ESS_MODULE_DIR}/bifrost/geometry/BifrostConfig.h
  IntervalTest.cpp)
create_test_executable(IntervalTest)


set(CaenConfigTest_INC
  ${ESS_MODULE_DIR}/caen/geometry/Config.h)
set(CaenConfigTest_SRC
  ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
  ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.cpp
  ${ESS_MODULE_DIR}/bifrost/geometry/BifrostConfig.cpp
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
  ${ESS_MODULE_DIR}/loki/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/readout/ess/Parser.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  )
set(CombinedParserTest_SRC
  CombinedParserTest.cpp
  ${ESS_MODULE_DIR}/loki/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/caen/readout/DataParser.cpp
  ${ESS_COMMON_DIR}/readout/ess/Parser.cpp
  ${ESS_COMMON_DIR}/testutils/SocketMock.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_test_executable(CombinedParserTest)

##
## Module test for amplitude mask used in Caen
##
set(AmplitudeMaskTest_INC
  ${ESS_MODULE_DIR}/caen/generators/ReadoutGenerator.h
  ${ESS_MODULE_DIR}/caen/readout/DataParser.h
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.h
  ${ESS_COMMON_DIR}/testutils/SocketMock.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
)
set(AmplitudeMaskTest_SRC
  AmplitudeMaskTest.cpp
  ${ESS_MODULE_DIR}/caen/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/caen/readout/DataParser.cpp
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_test_executable(AmplitudeMaskTest)
