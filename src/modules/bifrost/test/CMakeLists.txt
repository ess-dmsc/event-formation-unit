#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

##
## Pure unit tests
##

set(BifrostGeometryTest_INC
    ../geometry/BifrostConfig.h
    ../geometry/BifrostGeometry.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
    ${ESS_MODULE_DIR}/caen/geometry/Interval.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.h
)
set(BifrostGeometryTest_SRC
    BifrostGeometryTest.cpp
    ../geometry/BifrostConfig.cpp
    ../geometry/BifrostGeometry.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.cpp
)
create_test_executable(BifrostGeometryTest)

# New test for BifrostConfig
set(BifrostConfigTest_INC
    ../geometry/BifrostConfig.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.h
)
set(BifrostConfigTest_SRC
    BifrostConfigTest.cpp
    ../geometry/BifrostConfig.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.cpp
)
create_test_executable(BifrostConfigTest)
