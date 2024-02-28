#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(TTLMON_FULL "${ESS_MODULE_DIR}/cbm/configs/freiamon.json" ABSOLUTE)

##
## Pure unit tests
##

set(CbmParserTest_INC
  ../geometry/Parser.h)
set(CbmParserTest_SRC
  ../geometry/Parser.cpp
  ParserTest.cpp)
create_test_executable(CbmParserTest)

set(CbmConfigTest_INC
  ../geometry/Config.h)
set(CbmConfigTest_SRC
  ../geometry/Config.cpp
  ConfigTest.cpp)
create_test_executable(CbmConfigTest)
target_compile_definitions(CbmConfigTest PRIVATE CBM_FULL="${CBM_FULL}")
