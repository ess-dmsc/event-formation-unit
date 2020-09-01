find_package(asio REQUIRED)
find_package(Trompeloeil REQUIRED)

set(AdcReadoutTest_SRC
    UnitTests.cpp
    CircularBufferTest.cpp
    AdcReadoutTest.cpp
    AdcParseTest.cpp
    AdcDataProcessorTest.cpp
    SampleProcessingTest.cpp
    CLIArgumentsTest.cpp
    AdcTimeStampTest.cpp
    UDPClientTest.cpp
    DelayLineProducerTest.cpp
    DelayLineEventFormationTest.cpp
    PulseProcessingTest.cpp
    DelayLinePositionCalcTest.cpp
    PulseBufferTest.cpp
    EventSerializerTest.cpp
    EventBufferTest.cpp
    OffsetTimeTest.cpp
    ${ESS_SOURCE_DIR}/test/TestUDPServer.cpp
    )

set(AdcReadoutTest_INC ${ESS_SOURCE_DIR}/test/TestUDPServer.cpp)

list(APPEND AdcReadoutTest_SRC $<TARGET_OBJECTS:AdcReadout>)
create_test_executable(AdcReadoutTest SKIP_MEMGRIND)
target_include_directories(AdcReadoutTest PRIVATE ${Trompeloeil_INCLUDE_DIR})

get_filename_component(TEST_PACKET_PATH "test_packet_1.dat" DIRECTORY)
target_compile_definitions(AdcReadoutTest
    PRIVATE TEST_PACKET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_PACKET_PATH}/")
message(STATUS "Test ADC packets in path: ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_PACKET_PATH}/")

target_include_directories(AdcReadoutTest PRIVATE ${ASIO_INCLUDE_DIR} ${ESS_SOURCE_DIR})
