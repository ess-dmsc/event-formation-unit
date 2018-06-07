#=============================================================================
# Tests
#=============================================================================

set(SRSMappingsTest_SRC
    SRSMappingsTest.cpp
    SRSMappings.cpp
    )
create_test_executable(SRSMappingsTest)

set(SRSTimeTest_SRC
    SRSTimeTest.cpp
    SRSTime.cpp
    SRSTime.h
    )
create_test_executable(SRSTimeTest)
