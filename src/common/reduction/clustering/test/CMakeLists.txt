# \todo AbstractClusterer tests

set(GapClustererTest_SRC
  GapClustererTest.cpp
  )
create_test_executable(GapClustererTest)



set(Hierarchical2DClustererTest_SRC
  Hierarchical2DClustererTest.cpp
  )
create_test_executable(Hierarchical2DClustererTest)


#todo make abstract geometry class, non MG-specific?
set(GapClusterer2DTest_INC
  ${ESS_COMMON_DIR}/reduction/multigrid/ModuleGeometry.h
  )
set(GapClusterer2DTest_SRC
  GapClusterer2DTest.cpp
  ${ESS_COMMON_DIR}/reduction/multigrid/ModuleGeometry.cpp
  )
create_test_executable(GapClusterer2DTest)


set(Abstract2DClustererTest_INC
  ${ESS_COMMON_DIR}/reduction/clustering/Abstract2DClusterer.h
  )
set(Abstract2DClustererTest_SRC
  Abstract2DClustererTest.cpp
  ${ESS_COMMON_DIR}/reduction/clustering/Abstract2DClusterer.cpp
  )
create_test_executable(Abstract2DClustererTest)
