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


set(BuilderVMM3Test_INC
  BuilderVMM3.h
  CalibrationFile.h
  ParserVMM3.h
  SRSTime.h
  SRSMappings.h
  )
set(BuilderVMM3Test_SRC
  BuilderVMM3Test.cpp
  BuilderVMM3.cpp
  CalibrationFile.cpp
  ParserVMM3.cpp
  SRSTime.cpp
  SRSMappings.cpp
  )
create_test_executable(BuilderVMM3Test)


set(ParserVMM3Test_INC
  ParserVMM3TestData.h
  ParserVMM3.h
  SRSTime.h
  )
set(ParserVMM3Test_SRC
  ParserVMM3Test.cpp
  ParserVMM3.cpp
  SRSTime.cpp
  )
create_test_executable(ParserVMM3Test)


set(VMM3CalibrationFileTest_INC
  CalibrationFile.h
  ${ESS_SOURCE_DIR}/test/SaveBuffer.h
  )
set(VMM3CalibrationFileTest_SRC
  CalibrationFile.cpp
  CalibrationFileTest.cpp
  ${ESS_SOURCE_DIR}/test/SaveBuffer.cpp
  )
create_test_executable(VMM3CalibrationFileTest)
