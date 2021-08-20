
add_subdirectory(generators)

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
