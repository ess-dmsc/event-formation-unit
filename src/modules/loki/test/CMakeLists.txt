#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(LOKI_CONFIG "${ESS_MODULE_DIR}/loki/configs/LokiFull.json" ABSOLUTE)

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
)
set(LokiGeometryTest_SRC
    LokiGeometryTest.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiGeometry.cpp
)
create_test_executable(LokiGeometryTest)


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
