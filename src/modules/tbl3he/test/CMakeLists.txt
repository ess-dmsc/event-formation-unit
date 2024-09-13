#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

##
## Pure unit tests
##


# Testing geometry class for tbl3he
set(Tbl3GeometryTest_INC
  ${ESS_MODULE_DIR}/caen/geometry/Config.h
  ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
  ${ESS_MODULE_DIR}/caen/geometry/Interval.h
  ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.h
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeGeometry.h
  )
set(Tbl3GeometryTest_SRC
  ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
  ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
  ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
  ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.cpp
  ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeGeometry.cpp

  GeometryTest.cpp
  )
create_test_executable(Tbl3GeometryTest)
