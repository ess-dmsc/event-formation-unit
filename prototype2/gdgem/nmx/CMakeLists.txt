#=============================================================================
# Tests
#=============================================================================

set(ReadoutFileTest_SRC
    ReadoutFileTest.cpp
    ReadoutFile.cpp
    )
set(ReadoutFileTest_INC
    ReadoutFile.h
    Readout.h
    )
create_test_executable(ReadoutFileTest SKIP_MEMGRIND)

set(HitTest_SRC
    HitTest.cpp
    Hit.cpp
    )
set(HitTest_INC
    Hit.h
    )
create_test_executable(HitTest)

set(HitFileTest_SRC
    HitFileTest.cpp
    HitFile.cpp
    Hit.cpp
    )
set(HitFileTest_INC
    HitFile.h
    Hit.h
    )
create_test_executable(HitFileTest SKIP_MEMGRIND)

set(ClusterTest_SRC
    ClusterTest.cpp
    Cluster.cpp
    )
set(EventTest_INC
    Hit.h
    Cluster.h
    )
create_test_executable(ClusterTest)

set(EventTest_SRC
    EventTest.cpp
    Cluster.cpp
    Event.cpp
    )
set(EventTest_INC
    Hit.h
    Cluster.h
    Event.h
    )
create_test_executable(EventTest)

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
