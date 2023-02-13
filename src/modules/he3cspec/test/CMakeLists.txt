#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CAEN_FULL "${ESS_MODULE_DIR}/loki/configs/Loki.json" ABSOLUTE)

##
## Pure unit tests
##

set(He3cspecGeometryTest_INC
    ../geometry/He3cspecGeometry.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/Calibration.h
)
set(He3cspecGeometryTest_SRC
    He3cspecGeometryTest.cpp
    ../geometry/He3cspecGeometry.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Calibration.cpp
)
create_test_executable(He3cspecGeometryTest)

