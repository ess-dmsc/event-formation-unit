#=============================================================================
# Tests
#=============================================================================

set(HitTest_SRC
  HitTest.cpp
  )
create_test_executable(HitTest SKIP_MEMGRIND)

set(ClusterTest_SRC
  ClusterTest.cpp
  )
create_test_executable(ClusterTest)

set(GapClustererTest_SRC
  GapClustererTest.cpp
  )
create_test_executable(GapClustererTest)

set(EventTest_SRC
  EventTest.cpp
  )
create_test_executable(EventTest)

set(AbstractMatcherTest_SRC
  AbstractMatcherTest.cpp
  )
create_test_executable(AbstractMatcherTest)

set(EndMatcherTest_SRC
  EndMatcherTest.cpp
  )
create_test_executable(EndMatcherTest)

set(OverlapMatcherTest_SRC
  OverlapMatcherTest.cpp
  )
create_test_executable(OverlapMatcherTest)

set(GapMatcherTest_SRC
  GapMatcherTest.cpp
  )
create_test_executable(GapMatcherTest)
