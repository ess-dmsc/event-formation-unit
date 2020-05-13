find_package(asio REQUIRED)
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
    EventSerializer.cpp
    EventBuffer.cpp
    OffsetTime.cpp
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
    EventSerializer.h
    EventData.h
    EventBuffer.h
    OffsetTime.h
    )

create_object_module(AdcReadout)

add_subdirectory("integration_test")
add_subdirectory("test")
