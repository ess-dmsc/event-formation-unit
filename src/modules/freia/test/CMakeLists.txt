#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(FREIA_FULL "${ESS_MODULE_DIR}/freia/configs/freia.json" ABSOLUTE)

##
## Pure unit tests
##




set(FreiaGeometryTest_INC
  ../geometry/FreiaGeometry.h)
set(FreiaGeometryTest_SRC
  ../test/FreiaGeometryTest.cpp)
create_test_executable(FreiaGeometryTest)


set(AmorGeometryTest_INC
  ../geometry/AmorGeometry.h)
set(AmorGeometryTest_SRC
  ../test/AmorGeometryTest.cpp)
create_test_executable(AmorGeometryTest)
