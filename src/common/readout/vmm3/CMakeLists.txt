#=============================================================================
# ESS VMM3 Parser Tests
#=============================================================================

set(VMM3ParserTest_INC
  VMM3Parser.h
  test/VMM3ParserTestData.h
)
set(VMM3ParserTest_SRC
  test/VMM3ParserTest.cpp
  VMM3Parser.cpp
)
create_test_executable(VMM3ParserTest)


set(VMM3CalibrationTest_INC
  VMM3Calibration.h
)
set(VMM3CalibrationTest_SRC
  test/VMM3CalibrationTest.cpp
)
create_test_executable(VMM3CalibrationTest)

set(CalibFileTest_INC
  CalibFile.h
  Hybrid.h
)
set(CalibFileTest_SRC
  test/CalibFileTest.cpp
  CalibFile.cpp
)
create_test_executable(CalibFileTest)


set(HybridTest_INC
  Hybrid.h
)
set(HybridTest_SRC
  test/HybridTest.cpp
)
create_test_executable(HybridTest)
