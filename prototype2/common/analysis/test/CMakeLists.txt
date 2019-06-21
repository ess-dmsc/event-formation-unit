#=============================================================================
# Tests
#=============================================================================

set(AbstractAnalyzerTest_SRC
  AbstractAnalyzerTest.cpp
  )
create_test_executable(AbstractAnalyzerTest)

set(uTPCTest_SRC
  uTPCTest.cpp
  )
create_test_executable(uTPCTest)

#todo make abstract geometry class, non MG-specific?
set(MGAnalysisTest_SRC
  MGAnalysisTest.cpp
  ../../../multigrid/geometry/ModuleGeometry.cpp
  ../../../multigrid/geometry/Filter.cpp
  )
create_test_executable(MGAnalysisTest)
