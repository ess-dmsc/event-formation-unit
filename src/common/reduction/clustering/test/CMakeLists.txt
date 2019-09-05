# \todo AbstractClusterer tests

set(GapClustererTest_SRC
  GapClustererTest.cpp
  )
create_test_executable(GapClustererTest)

#todo make abstract geometry class, non MG-specific?
set(GapClusterer2DTest_SRC
  GapClusterer2DTest.cpp
  ../../../../multigrid/reduction/ModuleGeometry.cpp
  )
create_test_executable(GapClusterer2DTest)
