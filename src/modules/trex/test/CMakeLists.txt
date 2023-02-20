#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(TREX_FULL "${ESS_MODULE_DIR}/trex/configs/trex.json" ABSOLUTE)

##
## Pure unit tests
##

set(TREXConfigTest_INC
  ../geometry/Config.h)
set(TREXConfigTest_SRC
  ../geometry/Config.cpp
  ConfigTest.cpp)
create_test_executable(TREXConfigTest)
target_compile_definitions(TREXConfigTest PRIVATE TREX_FULL="${TREX_FULL}")




set(TREXGeometryTest_INC
        ../geometry/TREXGeometry.h
  ../geometry/Geometry.h)
set(TREXGeometryTest_SRC
  ../test/TREXGeometryTest.cpp
        ../geometry/TREXGeometry.cpp
  ../geometry/Geometry.h)
create_test_executable(TREXGeometryTest)


set(LETGeometryTest_INC
  ../geometry/LETGeometry.h
  ../geometry/Geometry.h)
set(LETGeometryTest_SRC
  ../test/LETGeometryTest.cpp
  ../geometry/LETGeometry.cpp
  ../geometry/Geometry.h)
create_test_executable(LETGeometryTest)


# set(TREXChannelMappingTest_INC
#   ../geometry/TREXChannelMapping.h)
# set(TREXChannelMappingTest_SRC
#   ../test/TREXChannelMappingTest.cpp)
# create_test_executable(TREXChannelMappingTest)
