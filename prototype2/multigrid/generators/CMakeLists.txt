#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

#set(mggen_readouts_SRC
#  ../../common/clustering/Hit.cpp
#  mggen_readouts.cpp
#  ../mgmesytec/MesytecReadout.cpp
#  ReaderReadouts.cpp
#  MGArgs.cpp
#  )
#set(mggen_readouts_INC
#  ../../common/clustering/Hit.h
#  ../mgmesytec/MesytecReadout.h
#  ReaderReadouts.h
#  MGArgs.h
#  )
#create_executable(mggen_readouts)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})

  set(ReadoutBuilderTest_SRC
    ReadoutBuilder.cpp
    ReadoutBuilderTest.cpp
    ReaderReadouts.cpp
    ../mgmesytec/MesytecReadout.cpp
    ../MgConfig.cpp
    )
  set(ReadoutBuilderTest_INC
    ReadoutBuilder.h
    ReaderReadouts.h
    ../../common/Buffer.h
    ../mgmesytec/MesytecReadout.h
    ../geometry/SequoiaGeometry.h
    ../MgConfig.h
    )
  set(ReadoutBuilderTest_LIB
    ${PCAP_LIBRARY}
    )
  create_test_executable(ReadoutBuilderTest)
  target_compile_definitions(ReadoutBuilderTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
  target_compile_definitions(ReadoutBuilderTest
    PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping ReadoutBuilder tests.")
endif()
