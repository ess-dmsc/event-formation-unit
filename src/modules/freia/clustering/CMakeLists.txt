#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

##
## Pure unit tests
##

set(MBEventBuilderTest_INC ../clustering/EventBuilder.h)
set(MBEventBuilderTest_SRC EventBuilderTest.cpp ../clustering/EventBuilder.cpp)
create_test_executable(MBEventBuilderTest)

## FP's reference data
set(MBReferenceDataTest_INC
  ../clustering/EventBuilder.h
  ReferenceDataTestData.h
)
set(MBReferenceDataTest_SRC
  ReferenceDataTest.cpp
  ReferenceDataTestData.cpp
  ../clustering/EventBuilder.cpp
)
create_test_executable(MBReferenceDataTest)
add_definitions("-I ${TEST_DATA_PATH}")
