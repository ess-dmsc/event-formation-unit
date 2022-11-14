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
    ${ESS_SOURCE_DIR}/modules/caen/geometry/Config.h
    ${ESS_SOURCE_DIR}/modules/caen/geometry/Calibration.h
)
set(LokiGeometryTest_SRC 
    LokiGeometryTest.cpp
    ../geometry/LokiGeometry.cpp
    ${ESS_SOURCE_DIR}/modules/caen/geometry/Config.cpp
    ${ESS_SOURCE_DIR}/modules/caen/geometry/Calibration.cpp
)
create_test_executable(LokiGeometryTest)


##
## Panel geometry
##
set(CaenPanelGeometryTest_INC
  ../geometry/LokiGeometry.h
  ../geometry/PanelGeometry.h)
set(CaenPanelGeometryTest_SRC PanelGeometryTest.cpp)
create_test_executable(CaenPanelGeometryTest)

##
## Validate LoKI geometry between EFU and Nexus Constructor
##
set(CaenFullGeometryTest_INC
  LokiFullGeometryTestData.h
  ../geometry/LokiGeometry.h
  ${ESS_SOURCE_DIR}/modules/caen/geometry/Calibration.h
  ../geometry/PanelGeometry.h)
set(LokiFullGeometryTest_SRC 
  LokiFullGeometryTest.cpp
  ${ESS_SOURCE_DIR}/modules/caen/geometry/Calibration.cpp)
create_test_executable(LokiFullGeometryTest)

