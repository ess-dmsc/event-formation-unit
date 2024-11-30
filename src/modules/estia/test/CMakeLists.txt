#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(ESTIA_FULL "${ESS_MODULE_DIR}/estia/configs/estia.json" ABSOLUTE)

##
## Pure unit tests
##

set(EstiaConfigTest_INC
  ../geometry/Config.h)
set(EstiaConfigTest_SRC
  ../geometry/Config.cpp
  ConfigTest.cpp)
create_test_executable(EstiaConfigTest)
target_compile_definitions(EstiaConfigTest PRIVATE ESTIA_FULL="${ESTIA_FULL}")



set(EstiaGeometryTest_INC
  ../geometry/Geometry.h)
set(EstiaGeometryTest_SRC
  ../test/GeometryTest.cpp)
create_test_executable(EstiaGeometryTest)


set(EstiaChannelMappingTest_INC
  ../geometry/EstiaChannelMapping.h)
set(EstiaChannelMappingTest_SRC
  ../test/EstiaChannelMappingTest.cpp)
create_test_executable(EstiaChannelMappingTest)
