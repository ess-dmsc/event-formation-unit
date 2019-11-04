#=============================================================================
# ESS Readout System Tests
#=============================================================================
set(ReadoutTest_SRC
  ReadoutTest.cpp
  Readout.cpp)
set(ReadoutTest_INC
  Readout.h
  ReadoutTestData.h)
create_test_executable(ReadoutTest)
