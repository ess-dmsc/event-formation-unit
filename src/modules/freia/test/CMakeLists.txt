#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(FREIA_FULL "${ESS_MODULE_DIR}/freia/configs/freia.json" ABSOLUTE)

##
## Pure unit tests
##




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
