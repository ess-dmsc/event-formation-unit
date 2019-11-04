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

# set(ReadoutDummyTest_SRC
#   ReadoutDummyTest.cpp
#   Readout.cpp
#   ReadoutDummy.cpp)
# set(ReadoutDummyTest_INC
#   Readout.h
#   ReadoutDummy.h
#   ReadoutTestData.h)
# create_test_executable(ReadoutDummyTest)
