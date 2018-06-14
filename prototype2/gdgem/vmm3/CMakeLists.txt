#=============================================================================
# Tests
#=============================================================================

set(ParserVMM3Test_SRC
    ParserVMM3Test.cpp
    ParserVMM3.cpp
    )
set(ParserVMM3Test_INC
    ParserVMM3TestData.h
    ParserVMM3.h
    )

create_test_executable(ParserVMM3Test)
