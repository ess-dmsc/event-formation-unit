#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(FREIA_FULL "${ESS_MODULE_DIR}/freia/configs/freia.json" ABSOLUTE)

##
## Pure unit tests
##

set(FreiaConfigTest_INC
  ../geometry/Config.h)
set(FreiaConfigTest_SRC
  ../geometry/Config.cpp
  ConfigTest.cpp)
create_test_executable(FreiaConfigTest)
target_compile_definitions(FreiaConfigTest PRIVATE FREIA_FULL="${FREIA_FULL}")



set(FreiaGeometryTest_INC
  ../geometry/FreiaGeometry.h
  ../geometry/Config.h)
set(FreiaGeometryTest_SRC
  ../geometry/Config.cpp
  ../geometry/GeometryBase.cpp
  ../test/FreiaGeometryTest.cpp)
create_test_executable(FreiaGeometryTest)


get_filename_component(AMOR_FULL "${ESS_MODULE_DIR}/freia/configs/amor.json" ABSOLUTE)

set(AmorGeometryTest_INC
  ../geometry/AmorGeometry.h
  ../geometry/Config.h)
set(AmorGeometryTest_SRC
  ../geometry/Config.cpp
  ../geometry/GeometryBase.cpp
  ../test/AmorGeometryTest.cpp)
create_test_executable(AmorGeometryTest)
target_compile_definitions(AmorGeometryTest PRIVATE AMOR_FULL="${AMOR_FULL}")

set(EstiaGeometryTest_INC
  ../geometry/EstiaGeometry.h
  ../geometry/Config.h)
set(EstiaGeometryTest_SRC
  ../geometry/Config.cpp
  ../geometry/GeometryBase.cpp
  ../test/EstiaGeometryTest.cpp)
create_test_executable(EstiaGeometryTest)

# Consolidated geometry tests (replaces removed selector based Geometry.h tests)
set(GeometryTest_INC
  ../geometry/FreiaGeometry.h
  ../geometry/AmorGeometry.h
  ../geometry/EstiaGeometry.h
  ../geometry/TBLMBGeometry.h
  ../geometry/GeometryBase.h)
set(GeometryTest_SRC
  ../geometry/Config.cpp
  ../geometry/GeometryBase.cpp
  ../test/GeometryTest.cpp)
create_test_executable(GeometryTest)
target_compile_definitions(GeometryTest PRIVATE UNIT_TEST)
