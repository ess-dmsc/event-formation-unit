# Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file

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
  VMM3Calibration.cpp
)
create_test_executable(VMM3CalibrationTest)


set(VMM3ConfigTest_INC
  VMM3Config.h
)
set(VMM3ConfigTest_SRC
  test/VMM3ConfigTest.cpp
  VMM3Config.cpp
)
create_test_executable(VMM3ConfigTest)

set(HybridTest_INC
  Hybrid.h
)
set(HybridTest_SRC
  test/HybridTest.cpp
)
create_test_executable(HybridTest)
