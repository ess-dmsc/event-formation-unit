/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "../DelayLineProducer.h"
#include "ev42_events_generated.h"
#include <gtest/gtest.h>
#include <ostream>
#include <trompeloeil.hpp>

using trompeloeil::_;
using namespace std::chrono_literals;

class DelayLineProducerStandIn : public DelayLineProducer {
public:
  explicit DelayLineProducerStandIn(AdcSettings Settings)
      : DelayLineProducer("no_broker", "no_topic", std::move(Settings)) {}
  void bypassMockSerializeAndSendEvent(DelayLineEvent const &Evt) {
    DelayLineProducer::serializeAndSendEvent(Evt);
  }
  MAKE_MOCK1(serializeAndSendEvent, void(DelayLineEvent const &), override);
  MAKE_MOCK2(produce, int(void *, size_t), override);
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
  TestProducer->bypassMockSerializeAndSendEvent(TestEvent);
  std::this_thread::sleep_for(50ms);
}

static std::uint16_t TestXpos = 1234;
static std::uint16_t TestYpos = 4321;
static std::uint32_t TestAmplitude = 135792;
static std::uint64_t TestTimestamp = 987654321;

bool dataHasExpectedContent(void *Ptr) {
  auto EventData = GetEventMessage(Ptr);
  std::uint32_t DetectorIDValue = (TestYpos << 16u) + TestXpos + 1u;
  EXPECT_EQ(EventData->pulse_time(), TestTimestamp);
  EXPECT_EQ(EventData->source_name()->str(),
            std::string("delay_line_detector"));
  EXPECT_EQ(EventData->message_id(), 0u);
  EXPECT_EQ(EventData->pulse_time(), TestTimestamp);
  EXPECT_EQ(EventData->detector_id()->size(), 1u);
  EXPECT_EQ(EventData->detector_id()->Get(0), DetectorIDValue);
  EXPECT_EQ(EventData->time_of_flight()->size(), 1u);
  EXPECT_EQ(EventData->time_of_flight()->Get(0), TestAmplitude);
  return true;
}

inline auto validFBEvent() {
  return trompeloeil::make_matcher<void *>(
      [](auto *Ptr) { return dataHasExpectedContent(Ptr); },
      [](std::ostream &Stream) {
        Stream
            << "Serialized buffer contents does not match expected contents.";
      });
}

TEST_F(DelayLineProducerTest, CallProduceContentTest) {
  REQUIRE_CALL(*TestProducer, produce(validFBEvent(), _)).TIMES(1).RETURN(0);
  DelayLineEvent TestEvent{};
  TestEvent.X = TestXpos;
  TestEvent.Y = TestYpos;
  TestEvent.Amplitude = TestAmplitude;
  TestEvent.Timestamp = TestTimestamp;
  TestProducer->bypassMockSerializeAndSendEvent(TestEvent);
  std::this_thread::sleep_for(50ms);
}
