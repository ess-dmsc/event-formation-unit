
#### Test
set(MGModuleGeometryTest_SRC ModuleGeometryTest.cpp)
set(MGModuleGeometryTest_LIB MgReductionLib)
create_test_executable(MGModuleGeometryTest)


#### Test
set(EventProcessingStatsTest_SRC EventProcessingStatsTest.cpp)
set(EventProcessingStatsTest_LIB MgReductionLib)
create_test_executable(EventProcessingStatsTest)


#### Test
set(MGModulePipelineTest_SRC ModulePipelineTest.cpp)
set(MGModulePipelineTest_INC ../ModulePipeline.h)
set(MGModulePipelineTest_LIB
  AnalysisLib
  MgGeometryLib
  MgMesytecLib
  MgReductionLib
  )
create_test_executable(MGModulePipelineTest)

set(TEST_DATA_PATH "${REFDATA}/multigrid/2018_09_03")
if(EXISTS ${TEST_DATA_PATH})

#### Test
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
    MgGeometryLib
    MgMesytecLib
    MgReductionLib
    )
  create_test_executable(MGReductionTest)
  target_compile_definitions(MGReductionTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "ECDC: No multigrid reference data found. Skipping MGReductionTests.")
endif()
