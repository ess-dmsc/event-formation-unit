#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

set(mggen_readouts_SRC
  ../../common/clustering/Hit.cpp
  mggen_readouts.cpp
  ../mesytec/Readout.cpp
  ReaderReadouts.cpp
  MGArgs.cpp
  )
set(mggen_readouts_INC
  ../../common/clustering/Hit.h
  ../mesytec/Readout.h
  ReaderReadouts.h
  MGArgs.h
  )
create_executable(mggen_readouts)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})
  set(MGBuilderReadoutsTest_SRC
    BuilderReadouts.cpp
    BuilderReadoutsTest.cpp
    ReaderReadouts.cpp
    ../mesytec/BuilderMesytec.cpp
    ../mesytec/Sis3153Parser.cpp
    ../mesytec/Vmmr16Parser.cpp
    ../mesytec/Readout.cpp
    ../reduction/EventAnalysis.cpp
    ../Config.cpp
    )
  set(MGBuilderReadoutsTest_INC
    BuilderReadouts.h
    ReaderReadouts.h
    ../../common/Buffer.h
    ../mesytec/BuilderMesytec.h
    ../mesytec/Sis3153Parser.h
    ../mesytec/Vmmr16Parser.h
    ../mesytec/Readout.h
    ../geometry/SequoiaGeometry.h
    ../Config.h
    )
  create_test_executable(MGBuilderReadoutsTest)
  target_compile_definitions(MGBuilderReadoutsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping BuilderReadouts tests.")
endif()
