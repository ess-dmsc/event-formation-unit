set(EventProcessingStatsTest_SRC
  EventProcessingStatsTest.cpp
  EventProcessingStats.cpp
  )
set(EventProcessingStatsTest_INC
  EventProcessingStats.h
  )
create_test_executable(EventProcessingStatsTest)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})
  set(MGModulePipelineTest_SRC
    ../geometry/Filter.cpp
    ../geometry/ChannelMappings.cpp
    ../geometry/DetectorMappings.cpp
    ../geometry/ModuleGeometry.cpp
    ../geometry/MGSeqMappings.cpp
    ../generators/ReaderReadouts.cpp
    ../mesytec/BuilderReadouts.cpp
    ../mesytec/Readout.cpp
    EventProcessingStats.cpp
    ModulePipeline.cpp
    ModulePipelineTest.cpp
    )
  set(MGModulePipelineTest_INC
    ../generators/ReaderReadouts.h
    ../mesytec/BuilderReadouts.h
    ../mesytec/Readout.h
    ../geometry/ChannelMappings.h
    ../geometry/DetectorMappings.h
    ../geometry/ModuleGeometry.h
    ../geometry/MGSeqMappings.h
    ../geometry/Filter.h
    EventProcessingStats.h
    ModulePipeline.h
    )
  create_test_executable(MGModulePipelineTest)
  target_compile_definitions(MGModulePipelineTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  set(MGReductionTest_SRC
    ../geometry/Filter.cpp
    ../geometry/ChannelMappings.cpp
    ../geometry/DetectorMappings.cpp
    ../geometry/ModuleGeometry.cpp
    ../geometry/MGSeqMappings.cpp
    ../generators/ReaderReadouts.cpp
    ../mesytec/BuilderReadouts.cpp
    ../mesytec/BuilderMesytec.cpp
    ../mesytec/Sis3153Parser.cpp
    ../mesytec/Vmmr16Parser.cpp
    ../mesytec/Readout.cpp
    ../Config.cpp
    EventProcessingStats.cpp
    ModulePipeline.cpp
    Reduction.cpp
    ReductionTest.cpp
    )
  set(MGReductionTest_INC
    ../generators/ReaderReadouts.h
    ../mesytec/BuilderReadouts.h
    ../mesytec/BuilderMesytec.h
    ../mesytec/Sis3153Parser.h
    ../mesytec/Vmmr16Parser.h
    ../mesytec/Readout.h
    ../geometry/ChannelMappings.h
    ../geometry/DetectorMappings.h
    ../geometry/ModuleGeometry.h
    ../geometry/MGSeqMappings.h
    ../geometry/Filter.h
    ../Config.h
    EventProcessingStats.h
    ModulePipeline.h
    Reduction.h
    )
  create_test_executable(MGReductionTest)
  target_compile_definitions(MGReductionTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "*** No multigrid reference data found. Skipping MGModulePipelineTest.")
  message(WARNING "*** No multigrid reference data found. Skipping MGReductionTests.")
endif()
