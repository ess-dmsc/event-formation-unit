#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(FREIA_FULL "${ESS_MODULE_DIR}/freia/configs/freia.json" ABSOLUTE)

##
## Pure unit tests
##

set(VMMConfigTest_INC
  ../geometry/Config.h)
set(VMMConfigTest_SRC
  ../geometry/Config.cpp
  ConfigTest.cpp)
create_test_executable(VMMConfigTest)
target_compile_definitions(VMMConfigTest PRIVATE FREIA_FULL="${FREIA_FULL}")

