set(MGHitTest_SRC
  HitTest.cpp
  Hit.cpp
  )
set(MGHitTest_INC
  Hit.h
  )
create_test_executable(MGHitTest SKIP_MEMGRIND)

set(Sis3153ParserTest_SRC
  Sis3153ParserTest.cpp
  Sis3153Parser.cpp
  )
set(Sis3153ParserTest_INC
  ../../common/Buffer.h
  Sis3153Parser.h
  TestData.h
  )
create_test_executable(Sis3153ParserTest)

set(Vmmr16ParserTest_SRC
  Vmmr16ParserTest.cpp
  Vmmr16Parser.cpp
  Sis3153Parser.cpp
  Hit.cpp
  )
set(Vmmr16ParserTest_INC
  ../../common/Buffer.h
  Sis3153Parser.h
  Vmmr16Parser.h
  Hit.h
  TestData.h
  )
create_test_executable(Vmmr16ParserTest)
