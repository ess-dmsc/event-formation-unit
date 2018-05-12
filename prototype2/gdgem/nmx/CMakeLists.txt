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
create_test_executable(ReadoutFileTest)

set(EventletTest_SRC
    EventletTest.cpp
    Eventlet.cpp
    )
set(EventletTest_INC
    Eventlet.h
    )
create_test_executable(EventletTest)

set(ClusterTest_SRC
    ClusterTest.cpp
    Cluster.cpp
    )
set(EventTest_INC
    Eventlet.h
    Cluster.h
    )
create_test_executable(ClusterTest)

set(EventTest_SRC
    EventTest.cpp
    Cluster.cpp
    Event.cpp
    )
set(EventTest_INC
    Eventlet.h
    Cluster.h
    Event.h
    )
create_test_executable(EventTest)

set(HistSerializerTest_SRC
    Hists.cpp
    HistSerializerTest.cpp
    HistSerializer.cpp
    )
set(HistSerializerTest_INC
    Eventlet.h
    Hists.h
    HistSerializer.h
    ../../common/Producer.h
    )
create_test_executable(HistSerializerTest)

set(TrackSerializerTest_SRC
    Eventlet.cpp
    Cluster.cpp
    Event.cpp
    TrackSerializerTest.cpp
    TrackSerializer.cpp
    )
set(TrackSerializerTest_INC
    Event.h
    Cluster.h
    Eventlet.h
    TrackSerializer.h
    )
create_test_executable(TrackSerializerTest)

