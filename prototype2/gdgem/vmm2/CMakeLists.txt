#=============================================================================
# Tests
#=============================================================================

# Unit Tests
set(EventFormationTest_SRC
    ../nmx/AbstractEventletBuilder.cpp
    ../nmx/Clusterer.cpp
    ../nmx/Event.cpp
    ../nmx/Hists.cpp
    ../srs/SRSTime.cpp
    ../srs/SRSMappings.cpp
    EventletBuilderSRS.cpp
    EventFormationTest.cpp
    ParserSRS.cpp
    )
set(EventFormationTest_INC
    ../nmx/AbstractEventletBuilder.h
    ../nmx/Clusterer.h
    ../nmx/Event.h
    ../nmx/Hists.h
    ../srs/SRSTime.h
    ../srs/SRSMappings.h
    EventletBuilderSRS.h
    ParserSRS.h
    )
create_test_executable(EventFormationTest)


set(EventletBuilderBenchmarkTest_SRC
    ../nmx/Clusterer.cpp
    ../nmx/Event.cpp ../nmx/Hists.cpp
    ../nmx/AbstractEventletBuilder.cpp
    ../srs/SRSTime.cpp
    ../srs/SRSMappings.cpp
    EventletBuilderSRSBenchmarkTest.cpp
    EventletBuilderSRS.cpp
    ParserSRS.cpp
    )
set(EventletBuilderBenchmarkTest_INC
    ../nmx/Clusterer.h
    ../nmx/Event.h
    ../nmx/Hists.h
    ../nmx/Eventlet.h
    ../srs/SRSTime.h
    ../srs/SRSMappings.h
    ParserSRS.h
    EventletBuilderSRS.h
    )
create_benchmark_executable(EventletBuilderBenchmarkTest)

set(ParserSRSTest_SRC
    ParserSRSTest.cpp
    ParserSRS.cpp
    )
set(ParserSRSTest_INC
    ParserSRS.h
    )
create_test_executable(ParserSRSTest)

set(EventletBuilderTest_SRC
    ../nmx/Clusterer.cpp
    ../nmx/Event.cpp
    ../nmx/Hists.cpp
    ../nmx/AbstractEventletBuilder.cpp
    ../srs/SRSTime.cpp
    ../srs/SRSMappings.cpp
    EventletBuilderSRSTest.cpp
    EventletBuilderSRS.cpp
    ParserSRS.cpp
    )
set(EventletBuilderTest_INC
    ../nmx/Clusterer.h
    ../nmx/Event.h
    ../nmx/Hists.h
    ../nmx/Eventlet.h
    ../srs/SRSTime.h
    ../srs/SRSMappings.h
    ParserSRS.h
    EventletBuilderSRS.h
    )
create_test_executable(EventletBuilderTest)
