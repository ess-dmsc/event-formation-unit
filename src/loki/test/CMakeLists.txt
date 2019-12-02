#=============================================================================
# Tests
#=============================================================================

##
## Pure unit tests
##

##
## geometry/
##

set(TubeAmpsTest_INC ../geometry/TubeAmps.h)
set(TubeAmpsTest_SRC TubeAmpsTest.cpp)
create_test_executable(TubeAmpsTest)

set(LokiPanelGeometryTest_INC
  ../geometry/TubeAmps.h
  ../geometry/PanelGeometry.h)
set(LokiPanelGeometryTest_SRC PanelGeometryTest.cpp)
create_test_executable(LokiPanelGeometryTest)


set(LokiCalibrationTest_INC
  ../../test/SaveBuffer.h
  ../geometry/Config.h
  ../geometry/Calibration.h)
set(LokiCalibrationTest_SRC
  ../geometry/Config.cpp
  ../geometry/Calibration.cpp
  ../../test/SaveBuffer.cpp
  CalibrationTest.cpp)
create_test_executable(LokiCalibrationTest)

set(LokiConfigTest_INC
  ../../test/SaveBuffer.h
  ../geometry/Config.h)
set(LokiConfigTest_SRC
  ../geometry/Config.cpp
  ../../test/SaveBuffer.cpp
  ConfigTest.cpp)
create_test_executable(LokiConfigTest)

##
## readout/
##
set(LokiDataParserTest_INC ../readout/DataParser.h ../../readout/ReadoutParser.h)
set(LokiDataParserTest_SRC ../readout/DataParser.cpp DataParserTest.cpp)
create_test_executable(LokiDataParserTest)


##
## Module Integration tests below
##


##
## Module integration test for both generic readout and LoKI
##
set(CombinedParserTest_INC ../readout/DataParser.h ../../readout/ReadoutParser.h)
set(CombinedParserTest_SRC ../../readout/ReadoutParser.cpp ../readout/DataParser.cpp
CombinedParserTest.cpp ReadoutGenerator.cpp)
create_test_executable(CombinedParserTest)
