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

set(NMXHitTest_SRC
    HitTest.cpp
    Hit.cpp
    )
set(NMXHitTest_INC
    Hit.h
    )
create_test_executable(NMXHitTest SKIP_MEMGRIND)

set(NMXClusterTest_SRC
    ClusterTest.cpp
    Cluster.cpp
    )
set(NMXEventTest_INC
    Hit.h
    Cluster.h
    )
create_test_executable(NMXClusterTest)

set(NMXEventTest_SRC
    EventTest.cpp
    Cluster.cpp
    Event.cpp
    )
set(NMXEventTest_INC
    Hit.h
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
    Hit.h
    ../../common/Hists.h
    ../../common/HistSerializer.h
    ../../common/Producer.h
    )
create_test_executable(HistSerializerTest)

set(TrackSerializerTest_SRC
    Hit.cpp
    Cluster.cpp
    Event.cpp
    TrackSerializerTest.cpp
    TrackSerializer.cpp
    )
set(TrackSerializerTest_INC
    Event.h
    Cluster.h
    Hit.h
    TrackSerializer.h
    )
create_test_executable(TrackSerializerTest)
