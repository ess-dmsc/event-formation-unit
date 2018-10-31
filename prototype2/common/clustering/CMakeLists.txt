#=============================================================================
# Tests
#=============================================================================

set(HitTest_SRC
  HitTest.cpp
  Hit.cpp
  )
set(HitTest_INC
  Hit.h
  )
create_test_executable(HitTest SKIP_MEMGRIND)

set(ClusterTest_SRC
  ClusterTest.cpp
  Cluster.cpp
  )
set(EventTest_INC
  Hit.h
  Cluster.h
  )
create_test_executable(ClusterTest)

set(ClusteringBase_SRC
  Cluster.cpp
  Hit.cpp
  )
set(ClusteringBase_INC
  Cluster.h
  Hit.h
  AbstractClusterer.h
  )

set(GapClustererTest_SRC
  ${ClusteringBase_SRC}
  GapClusterer.cpp
  GapClustererTest.cpp)
set(GapClustererTest_INC
  GapClusterer.h
  ${ClusteringBase_INC})
create_test_executable(GapClustererTest SKIP_MEMGRIND)
target_compile_definitions(GapClustererTest
  PRIVATE TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DATA_PATH}/")
