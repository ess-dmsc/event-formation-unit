#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

##
## Pure unit tests
##

set(BifrostGeometryTest_INC
    ../geometry/BifrostGeometry.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
    ${ESS_MODULE_DIR}/caen/geometry/Interval.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
)
set(BifrostGeometryTest_SRC
    BifrostGeometryTest.cpp
    ../geometry/BifrostGeometry.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
)
create_test_executable(BifrostGeometryTest)
