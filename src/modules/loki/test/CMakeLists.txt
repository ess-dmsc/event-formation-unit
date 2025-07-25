#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(LOKI_CONFIG "${ESS_MODULE_DIR}/loki/configs/LokiFull.json" ABSOLUTE)
get_filename_component(LOKI_CALIB "${ESS_MODULE_DIR}/loki/configs/lokinullcalib.json" ABSOLUTE)

##
## Pure unit tests
##

##
## Analog geometry - amplitude calculations
##
set(LokiGeometryTest_INC
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
    ${ESS_MODULE_DIR}/caen/geometry/Interval.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.h
    ${ESS_MODULE_DIR}/bifrost/geometry/BifrostConfig.h
)
set(LokiGeometryTest_SRC
    LokiGeometryTest.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.cpp
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.cpp
    ${ESS_MODULE_DIR}/bifrost/geometry/BifrostConfig.cpp
)
create_test_executable(LokiGeometryTest)



##
## Testing all banks from full config
##
set(LokiFullGeometryTest_INC
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
    ${ESS_MODULE_DIR}/caen/geometry/Interval.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.h
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.h
    ${ESS_MODULE_DIR}/bifrost/geometry/BifrostConfig.h
)
set(LokiFullGeometryTest_SRC
    LokiFullGeometryTest.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.cpp
    ${ESS_MODULE_DIR}/tbl3he/geometry/Tbl3HeConfig.cpp
    ${ESS_MODULE_DIR}/bifrost/geometry/BifrostConfig.cpp
)
create_test_executable(LokiFullGeometryTest)
target_compile_definitions(LokiFullGeometryTest PRIVATE LOKI_CONFIG="${LOKI_CONFIG}")
target_compile_definitions(LokiFullGeometryTest PRIVATE LOKI_CALIB="${LOKI_CALIB}")


##
## LOKI config test
##
set(LokiConfigTest_INC
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
)
set(LokiConfigTest_SRC
    LokiConfigTest.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp

)
create_test_executable(LokiConfigTest)
target_compile_definitions(LokiConfigTest PRIVATE LOKI_CONFIG="${LOKI_CONFIG}")
