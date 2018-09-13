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


set(CalibrationFileTest_SRC
    CalibrationFile.cpp
    CalibrationFileTest.cpp
    )

set(CalibrationFileTest_INC
    CalibrationFile.h
    )
create_test_executable(CalibrationFileTest)
