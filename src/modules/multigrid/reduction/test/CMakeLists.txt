set(MGModuleGeometryTest_SRC
  ModuleGeometryTest.cpp
  )
set(MGModuleGeometryTest_LIB
  MgReduction
  )
create_test_executable(MGModuleGeometryTest)

set(EventProcessingStatsTest_SRC
  EventProcessingStatsTest.cpp
  )
set(EventProcessingStatsTest_LIB
  MgReduction
  )
create_test_executable(EventProcessingStatsTest)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})
  set(MGModulePipelineTest_SRC
    ModulePipelineTest.cpp
    ../../generators/ReaderReadouts.cpp
    )
  set(MGModulePipelineTest_INC
    ../../generators/ReaderReadouts.h
    )
  set(MGModulePipelineTest_LIB
    MgGeometry
    MgMesytec
    MgReduction
    )
  create_test_executable(MGModulePipelineTest)
  target_compile_definitions(MGModulePipelineTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  set(MGReductionTest_SRC
    ReductionTest.cpp
    ../../generators/ReaderReadouts.cpp
    ../../Config.cpp
    )
  set(MGReductionTest_INC
    ../../generators/ReaderReadouts.h
    ../../Config.h
    )
  set(MGReductionTest_LIB
    MgGeometry
    MgMesytec
    MgReduction
    )
  create_test_executable(MGReductionTest)
  target_compile_definitions(MGReductionTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "ECDC: No multigrid reference data found. Skipping MGModulePipelineTest.")
  message(WARNING "ECDC: No multigrid reference data found. Skipping MGReductionTests.")
endif()
