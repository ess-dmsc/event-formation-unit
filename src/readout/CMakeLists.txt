#=============================================================================
# ESS Readout System Tests
#=============================================================================

set(ReadoutTest_INC Readout.h ReadoutTestData.h)
set(ReadoutTest_SRC ReadoutTest.cpp Readout.cpp)
create_test_executable(ReadoutTest)

#
set(ESSTimeTest_INC ESSTime.h)
set(ESSTimeTest_SRC ESSTimeTest.cpp)
create_test_executable(ESSTimeTest)
