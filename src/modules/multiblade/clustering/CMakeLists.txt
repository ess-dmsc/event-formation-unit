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

set(MBReferenceDataTest_INC ../clustering/EventBuilder.h)
set(MBReferenceDataTest_SRC ReferenceDataTest.cpp ../clustering/EventBuilder.cpp)
create_test_executable(MBReferenceDataTest)
