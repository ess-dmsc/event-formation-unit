#=============================================================================
# Tests
#=============================================================================

set(LokiDataParserTest_INC
  DataParser.h
  ../../readout/Readout.h
)
set(LokiDataParserTest_SRC
  DataParser.cpp
  DataParserTest.cpp
)

create_test_executable(LokiDataParserTest)
