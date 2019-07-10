//
// Created by Jonas Nilsson on 2019-07-09.
//

#include "../EventSerializer.h"
#include "ev42_events_generated.h"
#include <chrono>
#include <common/Producer.h>
#include <gtest/gtest.h>
#include <trompeloeil.hpp>

using trompeloeil::_;

using namespace std::chrono_literals;

class ProducerStandIn : public ProducerBase {
public:
  ProducerStandIn() = default;
  MAKE_MOCK2(produce, int(nonstd::span<const std::uint8_t>, std::int64_t),
             override);
};

class EventSerialisationTest : public ::testing::Test {
public:
  void SetUp() override { Producer = std::make_unique<ProducerStandIn>(); }
  std::unique_ptr<ProducerBase> Producer;
};

TEST_F(EventSerialisationTest, ProduceFlatbuffer_1) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .TIMES(1)
      .RETURN(0);
  {
    EventSerializer Serializer("SomeName", 1, 50ms,
                               dynamic_cast<ProducerBase *>(Producer.get()));
    Serializer.addEvent(std::make_unique<EventData>());
  }
}

bool checkFlatbuffer1(nonstd::span<const std::uint8_t> Buffer,
                      std::int64_t MessageTimestampMS) {
  auto EventMessage = GetEventMessage(Buffer.data());
  if (EventMessage->source_name()->str() != "SomeName") {
    return false;
  }
  if (EventMessage->message_id() != 0) {
    return false;
  }
  if (EventMessage->pulse_time() != 1000000) {
    return false;
  }
  if (EventMessage->time_of_flight()->size() != 1) {
    return false;
  }
  if (EventMessage->detector_id()->size() != 1) {
    return false;
  }
  EXPECT_EQ(MessageTimestampMS, 1);
  return true;
}

bool checkFlatbuffer2(nonstd::span<const std::uint8_t> Buffer,
                      std::int64_t MessageTimestampMS) {
  auto EventMessage = GetEventMessage(Buffer.data());
  if (EventMessage->source_name()->str() != "SomeName") {
    return false;
  }
  if (EventMessage->message_id() >= 2) {
    return false;
  }
  if (EventMessage->pulse_time() < 1000000 or
      EventMessage->pulse_time() > 1000003) {
    return false;
  }
  if (EventMessage->time_of_flight()->size() != 2) {
    return false;
  }
  if (EventMessage->detector_id()->size() != 2) {
    return false;
  }
  auto AdcDebugData = EventMessage->facility_specific_data_as_AdcPulseDebug();
  if (AdcDebugData->amplitude()->size() != 2) {
    return false;
  }
  if (AdcDebugData->peak_area()->size() != 2) {
    return false;
  }
  if (AdcDebugData->peak_time()->size() != 2) {
    return false;
  }
  if (AdcDebugData->background()->size() != 2) {
    return false;
  }
  if (AdcDebugData->threshold_time()->size() != 2) {
    return false;
  }
  if (EventMessage->detector_id()->size() != 2) {
    return false;
  }
  EXPECT_EQ(MessageTimestampMS, 1);
  return true;
}

TEST_F(EventSerialisationTest, ProduceFlatbuffer_2) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer1(_1, _2))
      .TIMES(1)
      .RETURN(0);
  {
    EventSerializer Serializer("SomeName", 1, 50ms,
                               dynamic_cast<ProducerBase *>(Producer.get()));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000000, 2, 3, 4, 5, 6000000, 7000000}));
  }
}

using namespace std::chrono_literals;

TEST_F(EventSerialisationTest, ProduceFlatbuffer_3) {
  bool CallDone = false;
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer1(_1, _2))
      .TIMES(1)
      .RETURN(0)
      .LR_SIDE_EFFECT(CallDone = true;);
  {
    EventSerializer Serializer("SomeName", 2, 60ms,
                               dynamic_cast<ProducerBase *>(Producer.get()));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000000, 2, 3, 4, 5, 6000000, 7000000}));
    std::this_thread::sleep_for(20ms);
    EXPECT_FALSE(CallDone);
    std::this_thread::sleep_for(100ms);
    EXPECT_TRUE(CallDone);
  }
}

TEST_F(EventSerialisationTest, ProduceFlatbuffer_4) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer2(_1, _2))
      .TIMES(2)
      .RETURN(0);
  {
    EventSerializer Serializer("SomeName", 2, 60ms,
                               dynamic_cast<ProducerBase *>(Producer.get()));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000000, 2, 3, 4, 5, 6000000, 7000000}));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000001, 2, 3, 4, 5, 6000000, 7000000}));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000002, 2, 3, 4, 5, 6000000, 7000000}));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000003, 2, 3, 4, 5, 6000000, 7000000}));
  }
}

TEST_F(EventSerialisationTest, ProduceFlatbuffer_5) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .TIMES(3)
      .RETURN(0);
  {
    EventSerializer Serializer("SomeName", 2, 60ms,
                               dynamic_cast<ProducerBase *>(Producer.get()));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000000, 2, 3, 4, 5, 6000000, 7000000}));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000001, 2, 3, 4, 5, 6000000, 7000000}));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000002, 2, 3, 4, 5, 6000000, 7000000}));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000003, 2, 3, 4, 5, 6000000, 7000000}));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000004, 2, 3, 4, 5, 6000000, 7000000}));
  }
}

bool checkFlatbuffer3(nonstd::span<const std::uint8_t> Buffer) {
  auto Verifier = flatbuffers::Verifier(Buffer.data(), Buffer.size());
  return VerifyEventMessageBuffer(Verifier);
}

TEST_F(EventSerialisationTest, ProduceFlatbuffer_6) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer3(_1))
      .TIMES(1)
      .RETURN(0);
  {
    EventSerializer Serializer("SomeName", 2, 60ms,
                               dynamic_cast<ProducerBase *>(Producer.get()));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000000, 2, 3, 4, 5, 6000000, 7000000}));
    Serializer.addEvent(std::unique_ptr<EventData>(
        new EventData{1000001, 2, 3, 4, 5, 6000000, 7000000}));
  }
}
