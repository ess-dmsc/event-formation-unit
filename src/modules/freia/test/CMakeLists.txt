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
  ${ESS_SOURCE_DIR}/test/SaveBuffer.h
  ../geometry/Config.h)
set(FreiaConfigTest_SRC
  ../geometry/Config.cpp
  ${ESS_SOURCE_DIR}/test/SaveBuffer.cpp
  ConfigTest.cpp)
create_test_executable(FreiaConfigTest)
target_compile_definitions(FreiaConfigTest PRIVATE FREIA_FULL="${FREIA_FULL}")
