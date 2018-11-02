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
  DelayLineProducerStandIn(AdcSettings Settings)
      : DelayLineProducer("no_broker", "no_topic", Settings) {}
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
  TestPulse.PeakTimestamp = {123456, 0};
  TestProducer->addPulse(TestPulse);
  std::this_thread::sleep_for(50ms);
}

TEST_F(DelayLineProducerTest, AddedInvalidPulse) {
  FORBID_CALL(*TestProducer, serializeAndSendEvent(_));
  PulseParameters TestPulse;
  TestPulse.Identifier.ChannelNr = 0;
  TestPulse.Identifier.SourceID = 1;
  TestPulse.PeakTimestamp = {123456, 0};
  TestProducer->addPulse(TestPulse);
  std::this_thread::sleep_for(50ms);
}

TEST_F(DelayLineProducerTest, CallProduceTest) {
  REQUIRE_CALL(*TestProducer, produce(_, _)).TIMES(1).RETURN(0);
  ;
  DelayLineEvent TestEvent;
  TestProducer->bypassMockSerializeAndSendEvent(TestEvent);
  std::this_thread::sleep_for(50ms);
}

static std::uint16_t TestXpos = 1234;
static std::uint16_t TestYpos = 4321;
static std::uint32_t TestAmplitude = 135792;
static std::uint64_t TestTimestamp = 987654321;

bool dataHasExpectedContent(void *Ptr) {
  auto EventData = GetEventMessage(Ptr);
  std::uint32_t DetectorIDValue = (TestXpos << 16) + TestYpos;
  if (EventData->pulse_time() != TestTimestamp or
      EventData->source_name()->str() != "delay_line_detector" or
      EventData->message_id() != 0 or
      EventData->pulse_time() != TestTimestamp or
      EventData->detector_id()->size() != 1 or
      EventData->detector_id()->operator[](0) != DetectorIDValue or
      EventData->time_of_flight()->size() != 1 or
      EventData->time_of_flight()->operator[](0) != TestAmplitude) {
    return false;
  }
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
  DelayLineEvent TestEvent;
  TestEvent.X = TestXpos;
  TestEvent.Y = TestYpos;
  TestEvent.Amplitude = TestAmplitude;
  TestEvent.Timestamp = TestTimestamp;
  TestProducer->bypassMockSerializeAndSendEvent(TestEvent);
  std::this_thread::sleep_for(50ms);
}
