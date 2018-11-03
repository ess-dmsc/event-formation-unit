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

set(GapClustererTest_SRC
  Cluster.cpp
  Hit.cpp
  GapClusterer.cpp
  GapClustererTest.cpp
  )
set(GapClustererTest_INC
  GapClusterer.h
  Cluster.h
  Hit.h
  AbstractClusterer.h
  )
create_test_executable(GapClustererTest)

set(EventTest_SRC
  Cluster.cpp
  Hit.cpp
  EventTest.cpp
  Event.cpp
  )
set(EventTest_INC
  Cluster.h
  Hit.h
  AbstractClusterer.h
  Event.h
  )
create_test_executable(EventTest)

set(MatcherTest_SRC
  Cluster.cpp
  Hit.cpp
  MatcherTest.cpp
  Event.cpp
  Matcher.cpp
  )
set(MatcherTest_INC
  Cluster.h
  Hit.h
  AbstractClusterer.h
  Event.h
  Matcher.h
  )
create_test_executable(MatcherTest)