set(MGReadoutTest_SRC
  ReadoutTest.cpp
  )
set(MGReadoutTest_LIB
  MgMesytec
  )
create_test_executable(MGReadoutTest SKIP_MEMGRIND)

set(MGSis3153ParserTest_SRC
  Sis3153ParserTest.cpp
  )
set(MGSis3153ParserTest_INC
  TestData.h
  )
set(MGSis3153ParserTest_LIB
  MgMesytec
  )
create_test_executable(MGSis3153ParserTest)

set(MGVmmr16ParserTest_SRC
  Vmmr16ParserTest.cpp
  )
set(MGVmmr16ParserTest_INC
  TestData.h
  )
set(MGVmmr16ParserTest_LIB
  MgMesytec
  )
create_test_executable(MGVmmr16ParserTest)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})
  set(MGBuilderReadoutsTest_SRC
    BuilderReadoutsTest.cpp
    ../../generators/ReaderReadouts.cpp
    )
  set(MGBuilderReadoutsTest_INC
    ../../generators/ReaderReadouts.h
    )
  set(MGBuilderReadoutsTest_LIB
    MgGeometry
    MgMesytec
    )
    create_test_executable(MGBuilderReadoutsTest)
  target_compile_definitions(MGBuilderReadoutsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "ECDC: No multigrid reference data found. Skipping BuilderReadouts tests.")
endif()

set(TEST_DATA_PATH2 "${REFDATA}/multigrid/2018_08_30")
if(EXISTS ${TEST_DATA_PATH2})
  set(MGBuilderMesytecTest_SRC
    ${ESS_SOURCE_DIR}/tools/ReaderPcap.cpp
    BuilderMesytecTest.cpp
    )
  set(MGBuilderMesytecTest_INC
    ${ESS_SOURCE_DIR}/tools/ReaderPcap.h
    TestData.h
    )
  set(MGBuilderMesytecTest_LIB
    ${PCAP_LIBRARY}
    MgGeometry
    MgMesytec
    )
  create_test_executable(MGBuilderMesytecTest)
  target_compile_definitions(MGBuilderMesytecTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH2}/")
  target_compile_definitions(MGBuilderMesytecTest
    PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}/")
else()
  message(WARNING "ECDC: No multigrid reference data found. Skipping BuilderMesytec tests.")
endif()
