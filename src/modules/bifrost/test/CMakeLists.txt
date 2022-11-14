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
    ../geometry/Config.h
    ../geometry/Calibration.h
)
set(BifrostGeometryTest_SRC 
    BifrostGeometryTest.cpp
    ../geometry/BifrostGeometry.cpp
    ../geometry/Config.cpp
    ../geometry/Calibration.cpp
)
create_test_executable(BifrostGeometryTest)

