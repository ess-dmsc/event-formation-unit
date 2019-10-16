#=============================================================================
# Tests
#=============================================================================

set(LokiGeometryTest_SRC GeometryTest.cpp)
set(LokiGeometryTest_INC ../geometry/Geometry.h)
create_test_executable(LokiGeometryTest)

set(HeliumTubeTest_SRC HeliumTubeTest.cpp)
set(HeliumTubeTest_INC ../geometry/HeliumTube.h)
create_test_executable(HeliumTubeTest)
