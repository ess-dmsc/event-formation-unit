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
  Sis3153Parser.h
  Vmmr16Parser.h
  Readout.h
  TestData.h
  )
create_test_executable(Vmmr16ParserTest)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})
  set(MGBuilderReadoutsTest_SRC
    BuilderReadoutsTest.cpp
    BuilderReadouts.cpp
    BuilderMesytec.cpp
    Sis3153Parser.cpp
    Vmmr16Parser.cpp
    Readout.cpp
    ../reduction/EventAnalysis.cpp
    ../generators/ReaderReadouts.cpp
    ../Config.cpp
    )
  set(MGBuilderReadoutsTest_INC
    BuilderReadouts.h
    BuilderMesytec.h
    Sis3153Parser.h
    Vmmr16Parser.h
    Readout.h
    ../geometry/SequoiaGeometry.h
    ../generators/ReaderReadouts.h
    ../Config.h
    )
  create_test_executable(MGBuilderReadoutsTest)
  target_compile_definitions(MGBuilderReadoutsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping BuilderReadouts tests.")
endif()


set(TEST_DATA_PATH2 "${REFDATA}/multigrid/2018_08_30")
if(EXISTS ${TEST_DATA_PATH2})
  set(BuilderMesytecTest_SRC
    BuilderMesytecTest.cpp
    BuilderReadouts.cpp
    BuilderMesytec.cpp
    Vmmr16Parser.cpp
    Sis3153Parser.cpp
    Readout.cpp
    ../../tools/ReaderPcap.cpp
    ../reduction/EventAnalysis.cpp
    ../Config.cpp
    )
  set(BuilderMesytecTest_INC
    BuilderMesytec.h
    Sis3153Parser.h
    Vmmr16Parser.h
    Readout.h
    TestData.h
    ../geometry/SequoiaGeometry.h
    ../../tools/ReaderPcap.h
    ../Config.h
    )
  set(BuilderMesytecTest_LIB
    ${PCAP_LIBRARY}
    )
  create_test_executable(BuilderMesytecTest)
  target_compile_definitions(BuilderMesytecTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH2}/")
  target_compile_definitions(BuilderMesytecTest
    PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping BuilderMesytec tests.")
endif()

