set(HitTest_SRC
  HitTest.cpp
  )
create_test_executable(HitTest SKIP_MEMGRIND)

set(ClusterTest_SRC
  ClusterTest.cpp
  )
create_test_executable(ClusterTest)

set(EventTest_SRC
  EventTest.cpp
  )
create_test_executable(EventTest)

set(ReducedEventTest_SRC
  ReducedEventTest.cpp
  )
create_test_executable(ReducedEventTest)

# \todo tests for NeutronEvent?