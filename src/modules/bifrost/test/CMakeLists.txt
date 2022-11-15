#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CAEN_FULL "${ESS_MODULE_DIR}/loki/configs/Loki.json" ABSOLUTE)

##
## Pure unit tests
##

set(BifrostGeometryTest_INC 
    ../geometry/BifrostGeometry.h
    ${ESS_SOURCE_DIR}/modules/caen/geometry/Config.h
    ${ESS_SOURCE_DIR}/modules/caen/geometry/Calibration.h
)
set(BifrostGeometryTest_SRC 
    BifrostGeometryTest.cpp
    ../geometry/BifrostGeometry.cpp
    ${ESS_SOURCE_DIR}/modules/caen/geometry/Config.cpp
    ${ESS_SOURCE_DIR}/modules/caen/geometry/Calibration.cpp
)
create_test_executable(BifrostGeometryTest)

