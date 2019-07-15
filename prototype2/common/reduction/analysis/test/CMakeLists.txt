set(AbstractAnalyzerTest_SRC
  AbstractAnalyzerTest.cpp
  )
create_test_executable(AbstractAnalyzerTest)

set(UtpcAnalyzerTest_SRC
  UtpcAnalyzerTest.cpp
  )
create_test_executable(UtpcAnalyzerTest)

#todo make abstract geometry class, non MG-specific?
set(MgAnalyzerTest_SRC
  MgAnalyzerTest.cpp
  ../../../../multigrid/geometry/ModuleGeometry.cpp
  ../../../../multigrid/geometry/Filter.cpp
  )
create_test_executable(MgAnalyzerTest)
