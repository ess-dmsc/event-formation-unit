set(MGHitTest_SRC
  ReadoutTest.cpp
  Readout.cpp
  )
set(MGHitTest_INC
  Readout.h
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
  Readout.cpp
  )
set(Vmmr16ParserTest_INC
  ../../common/Buffer.h
  Sis3153Parser.h
  Vmmr16Parser.h
  Readout.h
  TestData.h
  )
create_test_executable(Vmmr16ParserTest)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_08_30")
if(EXISTS ${TEST_DATA_PATH})
  set(BuilderMesytecTest_SRC
    ../generators/BuilderReadouts.cpp
    BuilderMesytec.cpp
    BuilderMesytecTest.cpp
    Vmmr16Parser.cpp
    Sis3153Parser.cpp
    Readout.cpp
    ../../tools/ReaderPcap.cpp
    ../reduction/EventAnalysis.cpp
    ../MgConfig.cpp
    )
  set(BuilderMesytecTest_INC
    BuilderMesytec.h
    ../../common/Buffer.h
    Sis3153Parser.h
    Vmmr16Parser.h
    Readout.h
    TestData.h
    ../geometry/SequoiaGeometry.h
    ../../tools/ReaderPcap.h
    ../MgConfig.h
    )
  set(BuilderMesytecTest_LIB
    ${PCAP_LIBRARY}
    )
  create_test_executable(BuilderMesytecTest)
  target_compile_definitions(BuilderMesytecTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
  target_compile_definitions(BuilderMesytecTest
    PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping relevant tests.")
endif()

