
set(HitTest_SRC
  HitTest.cpp
  )
create_test_executable(HitTest SKIP_MEMGRIND)

set(HitVectorTest_SRC
  HitVectorTest.cpp
  )
create_test_executable(HitVectorTest)

set(Hit2DVectorTest_SRC
  Hit2DVectorTest.cpp
  )
create_test_executable(Hit2DVectorTest)

set(ClusterTest_SRC
  ClusterTest.cpp
  )
create_test_executable(ClusterTest)

set(EventTest_SRC
  EventTest.cpp
  )
create_test_executable(EventTest)


set(Cluster2DTest_INC
  ${ESS_COMMON_DIR}/reduction/Cluster2D.h
  )
set(Cluster2DTest_SRC
  Cluster2DTest.cpp
  ${ESS_COMMON_DIR}/reduction/Cluster2D.cpp
  )
create_test_executable(Cluster2DTest)


set(ReducedEventTest_SRC
  ReducedEventTest.cpp
  )
create_test_executable(ReducedEventTest)

set(ChronoMergerTest_SRC
  ChronoMergerTest.cpp
  )
create_test_executable(ChronoMergerTest)

set(HitVectorBenchmark_SRC
  HitVectorBenchmark.cpp
  )
 create_benchmark_executable(HitVectorBenchmark)
