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
  ${ESS_MODULE_DIR}/freia/geometry/FreiaGeometry.h
  ${ESS_MODULE_DIR}/vmm/geometry/Config.h
  )
set(FreiaGeometryTest_SRC
  ${ESS_MODULE_DIR}/freia/test/FreiaGeometryTest.cpp
  ${ESS_MODULE_DIR}/freia/geometry/FreiaGeometry.cpp
  ${ESS_MODULE_DIR}/vmm/geometry/Config.cpp)
create_test_executable(FreiaGeometryTest)


set(AmorGeometryTest_INC
  ${ESS_MODULE_DIR}/freia/geometry/AmorGeometry.h
  ${ESS_MODULE_DIR}/vmm/geometry/Config.h)
set(AmorGeometryTest_SRC
  ${ESS_MODULE_DIR}/freia/test/AmorGeometryTest.cpp
  ${ESS_MODULE_DIR}/vmm/geometry/Config.cpp)
create_test_executable(AmorGeometryTest)
