#=============================================================================
# Tests
#=============================================================================

set(NMXReadoutTest_SRC
  ReadoutTest.cpp
  )
set(NMXReadoutTest_INC
  Readout.h
  )
create_test_executable(NMXReadoutTest SKIP_MEMGRIND)

set(NMXClusterTest_SRC
  ClusterTest.cpp
  Cluster.cpp
  )
set(NMXEventTest_INC
  ../../common/clustering/Hit.h
  Cluster.h
  )
create_test_executable(NMXClusterTest)

set(NMXEventTest_SRC
  EventTest.cpp
  Cluster.cpp
  Event.cpp
  )
set(NMXEventTest_INC
  ../../common/clustering/Hit.h
  Cluster.h
  Event.h
  )
create_test_executable(NMXEventTest)

set(HistSerializerTest_SRC
  ../../common/Hists.cpp
  ../../common/HistSerializerTest.cpp
  ../../common/HistSerializer.cpp
  )
set(HistSerializerTest_INC
  ../../common/clustering/Hit.h
  ../../common/Hists.h
  ../../common/HistSerializer.h
  ../../common/Producer.h
  )
create_test_executable(HistSerializerTest)

set(TrackSerializerTest_SRC
  ../../common/clustering/Hit.cpp
  Cluster.cpp
  Event.cpp
  TrackSerializerTest.cpp
  TrackSerializer.cpp
  )
set(TrackSerializerTest_INC
  ../../common/clustering/Hit.h
  Event.h
  Cluster.h
  TrackSerializer.h
  )
create_test_executable(TrackSerializerTest)
