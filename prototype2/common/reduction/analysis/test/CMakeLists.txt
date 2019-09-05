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
set(MgAnalyzerTest_SRC
  MgAnalyzerTest.cpp
  ../../../../multigrid/reduction/ModuleGeometry.cpp
  )
create_test_executable(MgAnalyzerTest)
