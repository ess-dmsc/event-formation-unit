#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CSPEC_FULL "${ESS_MODULE_DIR}/cspec/configs/cspec.json" ABSOLUTE)

##
## Pure unit tests
##

set(CspecGeometryTest_INC
    ../geometry/CspecGeometry.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
    ${ESS_MODULE_DIR}/caen/geometry/Interval.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
)
set(CspecGeometryTest_SRC
    CspecGeometryTest.cpp
    ../geometry/CspecGeometry.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
)
create_test_executable(CspecGeometryTest)
