#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

##
## Pure unit tests
##

set(MiraclesGeometryTest_INC
    ../geometry/MiraclesGeometry.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
    ${ESS_MODULE_DIR}/caen/geometry/Interval.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.h
    ${ESS_MODULE_DIR}/bifrost/geometry/BifrostConfig.h
)
set(MiraclesGeometryTest_SRC
    MiraclesGeometryTest.cpp
    ../geometry/MiraclesGeometry.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.cpp
    ${ESS_MODULE_DIR}/bifrost/geometry/BifrostConfig.cpp
)
create_test_executable(MiraclesGeometryTest)
