set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})
  set(MGReductionTest_SRC
    Reduction.cpp
    ReductionTest.cpp
    ../generators/BuilderReadouts.cpp
    ../generators/ReaderReadouts.cpp
    ../mesytec/Readout.cpp
    ../MgConfig.cpp
    EventAnalysis.cpp
    )
  set(MGReductionTest_INC
    Reduction.h
    ../generators/BuilderReadouts.h
    ../generators/ReaderReadouts.h
    ../../common/Buffer.h
    ../mesytec/Readout.h
    ../geometry/SequoiaGeometry.h
    ../MgConfig.h
    EventAnalysis.h
    )
  create_test_executable(MGReductionTest)
  target_compile_definitions(MGReductionTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping MGReduction tests.")
endif()

set(MGEventAnalysisTest_SRC
  ../../common/clustering/Hit.cpp
  ../../common/clustering/Event.cpp
  EventAnalysisTest.cpp
  EventAnalysis.cpp
  )
set(MGEventAnalysisTest_INC
  ../../common/clustering/Hit.h
  ../../common/clustering/Event.h
  ../geometry/SequoiaGeometry.h
  EventAnalysis.h
  )
create_test_executable(MGEventAnalysisTest)
