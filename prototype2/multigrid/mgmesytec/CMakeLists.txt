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

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_08_30")
if(EXISTS ${TEST_DATA_PATH})
  set(MesytecBuilderTest_SRC
    MesytecBuilder.cpp
    MesytecBuilderTest.cpp
    Vmmr16Parser.cpp
    Sis3153Parser.cpp
    MesytecReadout.cpp
    ../../tools/ReaderPcap.cpp
    ../MgConfig.cpp
    )
  set(MesytecBuilderTest_INC
    MesytecBuilder.h
    ../../common/Buffer.h
    Sis3153Parser.h
    Vmmr16Parser.h
    MesytecReadout.h
    TestData.h
    ../geometry/SequoiaGeometry.h
    ../../tools/ReaderPcap.h
    ../MgConfig.h
    )
  set(MesytecBuilderTest_LIB
    ${PCAP_LIBRARY}
    )
  create_test_executable(MesytecBuilderTest)
  target_compile_definitions(MesytecBuilderTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
  target_compile_definitions(MesytecBuilderTest
    PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping relevant tests.")
endif()

