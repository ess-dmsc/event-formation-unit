#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CAEN_FULL "${ESS_MODULE_DIR}/caen/configs/Loki.json" ABSOLUTE)

##
## Pure unit tests
##

set(MiraclesGeometryTest_INC 
    ../geometry/MiraclesGeometry.h
    ../geometry/Config.h
    ../geometry/Calibration.h
)
set(MiraclesGeometryTest_SRC 
    MiraclesGeometryTest.cpp
    ../geometry/MiraclesGeometry.cpp
    ../geometry/Config.cpp
    ../geometry/Calibration.cpp
)
create_test_executable(MiraclesGeometryTest)
