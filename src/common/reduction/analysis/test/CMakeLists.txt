set(AbstractAnalyzerTest_SRC
  AbstractAnalyzerTest.cpp
  )
create_test_executable(AbstractAnalyzerTest)

set(UtpcAnalyzerTest_SRC
  UtpcAnalyzerTest.cpp
  )
create_test_executable(UtpcAnalyzerTest)

set(EventAnalyzerTest_SRC
  EventAnalyzerTest.cpp
  )
create_test_executable(EventAnalyzerTest)


#todo make abstract geometry class, non MG-specific?
set(MgAnalyzerTest_INC
  ${ESS_COMMON_DIR}/reduction/multigrid/ModuleGeometry.h
  )
set(MgAnalyzerTest_SRC
  MgAnalyzerTest.cpp
  ${ESS_COMMON_DIR}/reduction/multigrid/ModuleGeometry.cpp
  )
create_test_executable(MgAnalyzerTest)
