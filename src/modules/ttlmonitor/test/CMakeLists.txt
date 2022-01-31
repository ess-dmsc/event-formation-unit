#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(TTLMON_FULL "${ESS_MODULE_DIR}/ttlmonitor/configs/freiamon.json" ABSOLUTE)

##
## Pure unit tests
##

set(TTLMonConfigTest_INC
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ../geometry/Config.h)
set(TTLMonConfigTest_SRC
  ../geometry/Config.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ConfigTest.cpp)
create_test_executable(TTLMonConfigTest)
target_compile_definitions(TTLMonConfigTest PRIVATE TTLMON_FULL="${TTLMON_FULL}")
