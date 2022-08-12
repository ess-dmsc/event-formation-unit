/** Copyright (C) 2018-2020 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "ev42_events_generated.h"
#include <adc_readout/DelayLineProducer.h>
#include <gtest/gtest.h>
#include <ostream>
#include <trompeloeil.hpp>

using trompeloeil::_;
using namespace std::chrono_literals;

class DelayLineProducerStandIn : public DelayLineProducer {
public:
  explicit DelayLineProducerStandIn(AdcSettings Settings)
      : DelayLineProducer("no_broker", "no_topic", std::move(Settings),
                          {OffsetTime::Offset::NONE}) {}
  void bypassMockSerializeAndSendEvent(DelayLineEvent const &Evt) {
    DelayLineProducer::serializeAndSendEvent(Evt);
  }
  MAKE_MOCK1(serializeAndSendEvent, void(DelayLineEvent const &), override);
  MAKE_MOCK2(produce, int(nonstd::span<const std::uint8_t>, std::int64_t),
             override);
  using DelayLineProducer::Serializer;
};

class DelayLineProducerTest : public ::testing::Test {
public:
  void SetUp() override {
    AdcSettings TempSettings;
    TempSettings.XAxis = AdcSettings::PositionSensingType::AMPLITUDE;
    TempSettings.ADC1Channel1 = AdcSettings::ChannelRole::AMPLITUDE_X_AXIS_1;
    TempSettings.YAxis = AdcSettings::PositionSensingType::CONST;
    TestProducer = std::make_unique<DelayLineProducerStandIn>(TempSettings);
  }
  std::unique_ptr<DelayLineProducerStandIn> TestProducer;
};

TEST_F(DelayLineProducerTest, NoPulseAdded) {
  FORBID_CALL(*TestProducer, serializeAndSendEvent(_));
  std::this_thread::sleep_for(50ms);
}

TEST_F(DelayLineProducerTest, AddedValidPulse) {
  REQUIRE_CALL(*TestProducer, serializeAndSendEvent(_)).TIMES(1);
  PulseParameters TestPulse;
  TestPulse.Identifier.ChannelNr = 0;
  TestPulse.Identifier.SourceID = 0;
  TestPulse.ThresholdTimestampNS = 12345;
  TestProducer->addPulse(TestPulse);
  std::this_thread::sleep_for(50ms);
}

TEST_F(DelayLineProducerTest, AddedInvalidPulse) {
  FORBID_CALL(*TestProducer, serializeAndSendEvent(_));
  PulseParameters TestPulse;
  TestPulse.Identifier.ChannelNr = 0;
  TestPulse.Identifier.SourceID = 1;
  TestProducer->addPulse(TestPulse);
  std::this_thread::sleep_for(50ms);
}

TEST_F(DelayLineProducerTest, CallProduceTest) {
  REQUIRE_CALL(*TestProducer, produce(_, _)).TIMES(1).RETURN(0);
  ;
  DelayLineEvent TestEvent{};
  TestProducer->Serializer.setTransmitTimeout(1ms);
  TestProducer->bypassMockSerializeAndSendEvent(TestEvent);
  std::this_thread::sleep_for(50ms);
}

static std::uint16_t TestXpos = 256;
static std::uint16_t TestYpos = 500;
static std::uint32_t TestAmplitude = 135792;
static std::uint64_t TestTimestamp = 987654321;

bool dataHasExpectedContent(nonstd::span<const std::uint8_t> Data) {
  auto EventData = GetEventMessage(Data.data());
  std::uint32_t DetectorIDValue = (TestYpos << 9u) + TestXpos + 1u;
  EXPECT_EQ(EventData->time_of_flight()->size(), 1u);
  EXPECT_EQ(EventData->pulse_time() + EventData->time_of_flight()->Get(0),
            TestTimestamp);
  EXPECT_EQ(EventData->source_name()->str(),
            std::string("delay_line_detector"));
  EXPECT_EQ(EventData->message_id(), 0u);
  EXPECT_EQ(EventData->detector_id()->size(), 1u);
  EXPECT_EQ(EventData->detector_id()->Get(0), DetectorIDValue);
  auto AdcData = EventData->facility_specific_data_as_AdcPulseDebug();
  EXPECT_EQ(AdcData->amplitude()->size(), 1u);
  EXPECT_EQ(AdcData->amplitude()->Get(0), TestAmplitude);
  return true;
}

TEST_F(DelayLineProducerTest, CallProduceContentTest) {
  REQUIRE_CALL(*TestProducer, produce(_, _))
      .WITH(dataHasExpectedContent(_1))
      .TIMES(1)
      .RETURN(0);
  TestProducer->Serializer.setTransmitTimeout(1ms);
  DelayLineEvent TestEvent{};
  TestEvent.X = TestXpos;
  TestEvent.Y = TestYpos;
  TestEvent.Amplitude = TestAmplitude;
  TestEvent.Timestamp = TestTimestamp;
  TestProducer->bypassMockSerializeAndSendEvent(TestEvent);
  std::this_thread::sleep_for(50ms);
}
