find_package(ASIO REQUIRED)
find_package(Trompeloeil REQUIRED)

set(AdcReadout_SRC
    AdcReadout.cpp
    AdcReadoutBase.cpp
    AdcParse.cpp
    PeakFinder.cpp
    AdcDataProcessor.cpp
    SampleProcessing.cpp
    AdcSettings.cpp
    AdcTimeStamp.cpp
    UDPClient.cpp
    DelayLineProducer.cpp
    DelayLineEventFormation.cpp
    DelayLineProcessing.cpp
    DelayLinePositionCalc.cpp
    PulseParameters.cpp
    PulseBuffer.cpp
    )

set(AdcReadout_INC
    AdcReadoutConstants.h
    AdcReadoutBase.h
    AdcParse.h
    AdcBufferElements.h
    CircularBuffer.h
    PeakFinder.h
    AdcDataProcessor.h
    SampleProcessing.h
    AdcSettings.h
    AdcTimeStamp.h
    UDPClient.h
    DelayLineProducer.h
    DelayLineEventFormation.h
    DelayLineProcessing.h
    DelayLinePositionCalc.h
    PulseParameters.h
    SamplingRun.h
    ChannelID.h
    PulseBuffer.h
    )

create_object_module(AdcReadout)

set(AdcReadoutTest_SRC
    test/UnitTests.cpp
    test/CircularBufferTest.cpp
    test/AdcReadoutTest.cpp
    test/AdcParseTest.cpp
    test/TestUDPServer.cpp
    test/AdcDataProcessorTest.cpp
    test/SampleProcessingTest.cpp
    test/CLIArgumentsTest.cpp
    test/AdcTimeStampTest.cpp
    test/UDPClientTest.cpp
    test/DelayLineProducerTest.cpp
    test/DelayLineEventFormationTest.cpp
    test/PulseProcessingTest.cpp
    test/DelayLinePositionCalcTest.cpp
    test/PulseBufferTest.cpp
    )

set(AdcReadoutTest_INC
    test/TestUDPServer.h
    )

set(AdcReadoutTest_LIB
    eventlib
    AdcReadout
    )

list(APPEND AdcReadoutTest_SRC ${EFU_MODULES})
create_test_executable(AdcReadoutTest SKIP_MEMGRIND)
target_include_directories(AdcReadoutTest PRIVATE ${Trompeloeil_INCLUDE_DIR})

get_filename_component(TEST_PACKET_PATH "test/test_packet_1.dat" DIRECTORY)
target_compile_definitions(AdcReadoutTest
    PRIVATE TEST_PACKET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_PACKET_PATH}/")
message(STATUS "Test ADC packets in path: ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_PACKET_PATH}/")

target_include_directories(AdcReadoutTest PRIVATE ${ASIO_INCLUDE_DIR})

add_subdirectory("integration_test")
