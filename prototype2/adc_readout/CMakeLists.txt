find_package(ASIO REQUIRED)

set(AdcReadoutBase_SRC
    AdcReadoutBase.cpp
    AdcParse.cpp
    PeakFinder.cpp
    AdcDataProcessor.cpp
    SampleProcessing.cpp
    AdcSettings.cpp
    AdcTimeStamp.cpp
    )

set(AdcReadoutBase_INC
    AdcReadoutConstants.h
    AdcReadoutBase.h
    AdcParse.h
    AdcBufferElements.h
    atomicops.h
    CircularBuffer.h
    readerwriterqueue.h
    PeakFinder.h
    AdcDataProcessor.h
    SampleProcessing.h
    AdcSettings.h
    AdcTimeStamp.h
    )


set(AdcReadout_SRC
    AdcReadout.cpp
    ${AdcReadoutBase_SRC}
    )

set(AdcReadout_INC
    ${AdcReadoutBase_INC}
    )

create_module(AdcReadout)

set(AdcReadoutTest_SRC
    test/UnitTests.cpp
    test/CircularBufferTest.cpp
    test/AdcReadoutTest.cpp
    test/AdcParseTest.cpp
    test/TestUDPServer.cpp
    test/PeakFinderTest.cpp
    test/AdcDataProcessorTest.cpp
    test/SampleProcessingTest.cpp
    test/CLIArgumentsTest.cpp
    test/AdcTimeStampTest.cpp
    ${AdcReadoutBase_SRC}
    )

set(AdcReadoutTest_INC
    test/TestUDPServer.h
    ${AdcReadoutBase_INC}
    )

set(AdcReadoutTest_LIB
    eventlib
    )

create_test_executable(AdcReadoutTest)

get_filename_component(TEST_PACKET_PATH "test/test_packet_1.dat" DIRECTORY)
target_compile_definitions(AdcReadoutTest
    PRIVATE TEST_PACKET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_PACKET_PATH}/")
message(STATUS "Test ADC packets in path: ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_PACKET_PATH}/")

target_include_directories(AdcReadoutTest PRIVATE ${ASIO_INCLUDE_DIR})

add_subdirectory("chopper_integration_test")
