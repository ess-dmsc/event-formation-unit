set(MGHitTest_SRC
  MesytecReadoutTest.cpp
  MesytecReadout.cpp
  )
set(MGHitTest_INC
  MesytecReadout.h
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
  MesytecReadout.cpp
  )
set(Vmmr16ParserTest_INC
  ../../common/Buffer.h
  Sis3153Parser.h
  Vmmr16Parser.h
  MesytecReadout.h
  TestData.h
  )
create_test_executable(Vmmr16ParserTest)

set(MesytecBuilderTest_SRC
  MesytecBuilder.cpp
  MesytecBuilderTest.cpp
  Vmmr16Parser.cpp
  Sis3153Parser.cpp
  MesytecReadout.cpp
  )
set(MesytecBuilderTest_INC
  MesytecBuilder.h
  ../../common/Buffer.h
  Sis3153Parser.h
  Vmmr16Parser.h
  MesytecReadout.h
  TestData.h
  ../geometry/SequoiaGeometry.h
  )
create_test_executable(MesytecBuilderTest)

