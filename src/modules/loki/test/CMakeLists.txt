#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CAEN_FULL "${ESS_MODULE_DIR}/loki/configs/Loki.json" ABSOLUTE)

##
## Pure unit tests
##

##
## Analog geometry - amplitude calculations
##
set(LokiGeometryTest_INC
    ../geometry/LokiGeometry.h
    ../geometry/PanelGeometry.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
    ${ESS_MODULE_DIR}/caen/geometry/Interval.h
)
set(LokiGeometryTest_SRC
    LokiGeometryTest.cpp
    ../geometry/LokiGeometry.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
)
create_test_executable(LokiGeometryTest)


##
## LOKI conifig test
##
set(LokiConfigTest_INC
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.h
)
set(LokiConfigTest_SRC
    LokiConfigTest.cpp
    ${ESS_MODULE_DIR}/loki/geometry/LokiConfig.cpp

)
create_test_executable(LokiConfigTest)


##
## Panel geometry
##
set(LokiPanelGeometryTest_INC
  ../geometry/LokiGeometry.h
  ../geometry/PanelGeometry.h)
set(LokiPanelGeometryTest_SRC PanelGeometryTest.cpp)
create_test_executable(LokiPanelGeometryTest)

##
## Validate LoKI geometry between EFU and Nexus Constructor
##
set(LokiFullGeometryTest_INC
  LokiFullGeometryTestData.h
  ../geometry/LokiGeometry.h
  ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.h
  ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
  ../geometry/PanelGeometry.h)
set(LokiFullGeometryTest_SRC
  LokiFullGeometryTest.cpp
  ${ESS_MODULE_DIR}/caen/geometry/CDCalibration.cpp
  ${ESS_MODULE_DIR}/caen/geometry/Interval.cpp
  )

create_test_executable(LokiFullGeometryTest)
