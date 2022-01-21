#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CSPEC_FULL "${ESS_MODULE_DIR}/cspec/configs/cspec.json" ABSOLUTE)

##
## Pure unit tests
##

set(CSPECConfigTest_INC
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ../geometry/Config.h)
set(CSPECConfigTest_SRC
  ../geometry/Config.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ConfigTest.cpp)
create_test_executable(CSPECConfigTest)
target_compile_definitions(CSPECConfigTest PRIVATE CSPEC_FULL="${CSPEC_FULL}")




# set(CSPECGeometryTest_INC
#   ../geometry/Geometry.h)
# set(CSPECGeometryTest_SRC
#   ../test/GeometryTest.cpp)
# create_test_executable(CSPECGeometryTest)


# set(CSPECChannelMappingTest_INC
#   ../geometry/CSPECChannelMapping.h)
# set(CSPECChannelMappingTest_SRC
#   ../test/CSPECChannelMappingTest.cpp)
# create_test_executable(CSPECChannelMappingTest)


