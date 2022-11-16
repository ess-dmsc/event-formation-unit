#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CAEN_FULL "${ESS_MODULE_DIR}/loki/configs/Loki.json" ABSOLUTE)

##
## Pure unit tests
##

set(MiraclesGeometryTest_INC 
    ../geometry/MiraclesGeometry.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/Calibration.h
)
set(MiraclesGeometryTest_SRC 
    MiraclesGeometryTest.cpp
    ../geometry/MiraclesGeometry.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Calibration.cpp
)
create_test_executable(MiraclesGeometryTest)
