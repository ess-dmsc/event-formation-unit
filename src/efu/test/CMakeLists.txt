#=============================================================================
# EFU Tests
#=============================================================================

#
set(HwCheckTest_INC
  ../HwCheck.h
)
set(HwCheckTest_SRC
  ../HwCheck.cpp
  HwCheckTest.cpp
)
create_test_executable(HwCheckTest)

#
set(ServerTest_INC
  ../Server.h
  ../Parser.h
)
set(ServerTest_SRC
  ServerTest.cpp
)
create_test_executable(ServerTest)

#
set(ParserTest_SRC
  ParserTest.cpp
)
set(ParserTest_INC
  ../Parser.h
  )
create_test_executable(ParserTest)
