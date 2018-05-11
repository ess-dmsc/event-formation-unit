#=============================================================================
# Tests
#=============================================================================

set(EventletTest_SRC
    EventletTest.cpp
    Eventlet.cpp
    )
set(EventletTest_INC
    Eventlet.h
    )
create_test_executable(EventletTest)

set(EventTest_SRC
    EventTest.cpp
    Event.cpp
    )
set(EventTest_INC
    Event.h
    )
create_test_executable(EventTest)

# Get rid of this. Use LogicalGeometry
set(GeometryTest_SRC
    GeometryTest.cpp
    Geometry.cpp
    )
set(GeometryTest_INC
    Geometry.h
    )
create_test_executable(GeometryTest)

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
    Event.cpp
    TrackSerializerTest.cpp
    TrackSerializer.cpp
    )
set(TrackSerializerTest_INC
    Event.h
    Eventlet.h
    TrackSerializer.h
    )
create_test_executable(TrackSerializerTest)

