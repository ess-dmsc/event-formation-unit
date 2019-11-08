#=============================================================================
# Tests
#=============================================================================

##
## Pure unit tests
##

##
## geometry/
##
set(LokiGeometryTest_SRC GeometryTest.cpp)
set(LokiGeometryTest_INC ../geometry/Geometry.h)
create_test_executable(LokiGeometryTest)

set(HeliumTubeTest_SRC HeliumTubeTest.cpp)
set(HeliumTubeTest_INC ../geometry/HeliumTube.h)
create_test_executable(HeliumTubeTest)

##
## readout/
##
set(LokiDataParserTest_INC ../readout/DataParser.h ../../readout/Readout.h)
set(LokiDataParserTest_SRC ../readout/DataParser.cpp DataParserTest.cpp)
create_test_executable(LokiDataParserTest)


##
## Module Integration tests below
##


##
## Module integration test for both generic readout and LoKI
##
set(CombinedParserTest_INC ../readout/DataParser.h ../../readout/Readout.h)
set(CombinedParserTest_SRC ../../readout/Readout.cpp ../readout/DataParser.cpp
  CombinedParserTest.cpp)
create_test_executable(CombinedParserTest)
