# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file


get_filename_component(
  VMM_CONFIG_FILE "${ESS_COMMON_DIR}/readout/vmm3/test/vmmconfig.json" ABSOLUTE
 )

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
target_compile_definitions(VMM3ConfigTest PRIVATE VMM_CONFIG_FILE="${VMM_CONFIG_FILE}")


set(HybridTest_INC
  Hybrid.h
)
set(HybridTest_SRC
  test/HybridTest.cpp
)
create_test_executable(HybridTest)
