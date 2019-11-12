#=============================================================================
# ESS Readout System Tests
#=============================================================================

set(ReadoutParserTest_INC ReadoutParser.h ReadoutParserTestData.h)
set(ReadoutParserTest_SRC ReadoutParserTest.cpp ReadoutParser.cpp)
create_test_executable(ReadoutParserTest)

#
set(ESSTimeTest_INC ESSTime.h)
set(ESSTimeTest_SRC ESSTimeTest.cpp)
create_test_executable(ESSTimeTest)
