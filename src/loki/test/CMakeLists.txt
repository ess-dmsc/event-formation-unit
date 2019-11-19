#=============================================================================
# Tests
#=============================================================================

##
## Pure unit tests
##

##
## geometry/
##

set(HeliumTubeTest_INC ../geometry/HeliumTube.h)
set(HeliumTubeTest_SRC HeliumTubeTest.cpp)
create_test_executable(HeliumTubeTest)

set(LokiPanelGeometryTest_INC
  ../geometry/HeliumTube.h
  ../geometry/PanelGeometry.h)
set(LokiPanelGeometryTest_SRC PanelGeometryTest.cpp)
create_test_executable(LokiPanelGeometryTest)


set(LokiConfigTest_INC ../geometry/Config.h)
set(LokiConfigTest_SRC
  ../geometry/Config.cpp
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
