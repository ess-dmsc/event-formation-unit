#=============================================================================
# Tests
#=============================================================================

set(SRSMappingsTest_SRC
    SRSMappingsTest.cpp
    SRSMappings.cpp
    )
create_test_executable(SRSMappingsTest)

set(SRSTimeTest_SRC
    SRSTimeTest.cpp
    SRSTime.cpp
    SRSTime.h
    )
create_test_executable(SRSTimeTest)

set(ParserVMM3Test_SRC
  ParserVMM3Test.cpp
  ParserVMM3.cpp
  SRSTime.cpp
  )
  
set(ParserVMM3Test_INC
  ParserVMM3TestData.h
  ParserVMM3.h
  SRSTime.h
  )
create_test_executable(ParserVMM3Test)

set(VMM3CalibrationFileTest_SRC
  CalibrationFile.cpp
  CalibrationFileTest.cpp
  )
set(VMM3CalibrationFileTest_INC
  CalibrationFile.h
  )
create_test_executable(VMM3CalibrationFileTest)

# todo BuilderVMM3 test needed
