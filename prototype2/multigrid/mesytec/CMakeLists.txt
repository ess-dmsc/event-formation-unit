set(MGReadoutTest_SRC
  ReadoutTest.cpp
  Readout.cpp
  )
set(MGReadoutTest_INC
  Readout.h
  )
create_test_executable(MGReadoutTest SKIP_MEMGRIND)

set(MGSis3153ParserTest_SRC
  Sis3153ParserTest.cpp
  Sis3153Parser.cpp
  )
set(MGSis3153ParserTest_INC
  Sis3153Parser.h
  TestData.h
  )
create_test_executable(MGSis3153ParserTest)

set(MGVmmr16ParserTest_SRC
  Vmmr16ParserTest.cpp
  Vmmr16Parser.cpp
  Sis3153Parser.cpp
  Readout.cpp
  )
set(MGVmmr16ParserTest_INC
  Sis3153Parser.h
  Vmmr16Parser.h
  Readout.h
  TestData.h
  )
create_test_executable(MGVmmr16ParserTest)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})
  set(MGBuilderReadoutsTest_SRC
    BuilderReadoutsTest.cpp
    BuilderReadouts.cpp
    BuilderMesytec.cpp
    Sis3153Parser.cpp
    Vmmr16Parser.cpp
    Readout.cpp
    ../geometry/Filter.cpp
    ../geometry/ChannelMappings.cpp
    ../geometry/MGSeqMappings.cpp
    ../geometry/DetectorMappings.cpp
    ../geometry/ModuleGeometry.cpp
    ../reduction/Reduction.cpp
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
    ../geometry/Filter.h
    ../geometry/ChannelMappings.h
    ../geometry/MGSeqMappings.h
    ../geometry/MG24Mappings.h
    ../geometry/DetectorMappings.h
    ../geometry/ModuleGeometry.h
    ../generators/ReaderReadouts.h
    ../Config.h
    ../reduction/Reduction.h
    ../reduction/EventAnalysis.h
    )
  create_test_executable(MGBuilderReadoutsTest)
  target_compile_definitions(MGBuilderReadoutsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping BuilderReadouts tests.")
endif()


set(TEST_DATA_PATH2 "${REFDATA}/multigrid/2018_08_30")
if(EXISTS ${TEST_DATA_PATH2})
  set(MGBuilderMesytecTest_SRC
    BuilderMesytecTest.cpp
    BuilderReadouts.cpp
    BuilderMesytec.cpp
    Vmmr16Parser.cpp
    Sis3153Parser.cpp
    Readout.cpp
    ../geometry/Filter.cpp
    ../geometry/ChannelMappings.cpp
    ../geometry/MGSeqMappings.cpp
    ../geometry/DetectorMappings.cpp
    ../geometry/ModuleGeometry.cpp
    ../../tools/ReaderPcap.cpp
    ../reduction/EventAnalysis.cpp
    ../reduction/Reduction.cpp
    ../Config.cpp
    )
  set(MGBuilderMesytecTest_INC
    BuilderMesytec.h
    Sis3153Parser.h
    Vmmr16Parser.h
    Readout.h
    TestData.h
    ../geometry/Filter.h
    ../geometry/ChannelMappings.h
    ../geometry/MGSeqMappings.h
    ../geometry/MG24Mappings.h
    ../geometry/DetectorMappings.h
    ../geometry/ModuleGeometry.h
    ../../tools/ReaderPcap.h
    ../Config.h
    ../reduction/Reduction.h
    ../reduction/EventAnalysis.h
    )
  set(MGBuilderMesytecTest_LIB
    ${PCAP_LIBRARY}
    )
  create_test_executable(MGBuilderMesytecTest)
  target_compile_definitions(MGBuilderMesytecTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH2}/")
  target_compile_definitions(MGBuilderMesytecTest
    PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping BuilderMesytec tests.")
endif()

