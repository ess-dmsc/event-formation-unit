#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(FREIA_FULL "${ESS_MODULE_DIR}/freia/configs/freia.json" ABSOLUTE)

##
## Pure unit tests
##

set(FreiaConfigTest_INC
  ../geometry/Config.h)
set(FreiaConfigTest_SRC
  ../geometry/Config.cpp
  ConfigTest.cpp)
create_test_executable(FreiaConfigTest)
target_compile_definitions(FreiaConfigTest PRIVATE FREIA_FULL="${FREIA_FULL}")



set(FreiaGeometryTest_INC
  ../geometry/Geometry.h)
set(FreiaGeometryTest_SRC
  ../test/GeometryTest.cpp)
create_test_executable(FreiaGeometryTest)


set(FreiaChannelMappingTest_INC
  ../geometry/FreiaChannelMapping.h)
set(FreiaChannelMappingTest_SRC
  ../test/FreiaChannelMappingTest.cpp)
create_test_executable(FreiaChannelMappingTest)


set(AMORChannelMappingTest_INC
  ../geometry/AMORChannelMapping.h)
set(AMORChannelMappingTest_SRC
  ../test/AMORChannelMappingTest.cpp)
create_test_executable(AMORChannelMappingTest)


set(EstiaChannelMappingTest_INC
  ../geometry/FreiaChannelMapping.h)
set(EstiaChannelMappingTest_SRC
  ../test/EstiaChannelMappingTest.cpp)
create_test_executable(EstiaChannelMappingTest)
