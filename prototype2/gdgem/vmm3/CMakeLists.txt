#=============================================================================
# Tests
#=============================================================================

set(ParserSRSVMM3Test_SRC
    ParserSRSTest.cpp
    ParserSRS.cpp
    )
set(ParserSRSVMM3Test_INC
    ParserSRSTestData.h
    ParserSRS.h
    )

create_test_executable(ParserSRSVMM3Test)
