#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(NMX_FULL "${ESS_MODULE_DIR}/nmx/configs/nmx.json" ABSOLUTE)
get_filename_component(NMX_MINI "${ESS_MODULE_DIR}/nmx/configs/mininmx.json" ABSOLUTE)

##
## Pure unit tests
##

set(NMXConfigTest_INC
  ../geometry/Config.h)
set(NMXConfigTest_SRC
  ../geometry/Config.cpp
  NMXConfigTest.cpp)
create_test_executable(NMXConfigTest)
target_compile_definitions(NMXConfigTest PRIVATE NMX_FULL="${NMX_FULL}")


set(NMXGeometryTest_INC
  ../geometry/NMXGeometry.h
  ../geometry/Geometry.h)
set(NMXGeometryTest_SRC
  ../test/NMXGeometryTest.cpp
  ../geometry/NMXGeometry.cpp
  ../geometry/Geometry.h)
create_test_executable(NMXGeometryTest)



# set(NMXChannelMappingTest_INC
#   ../geometry/NMXChannelMapping.h)
# set(NMXChannelMappingTest_SRC
#   ../test/NMXChannelMappingTest.cpp)
# create_test_executable(NMXChannelMappingTest)
