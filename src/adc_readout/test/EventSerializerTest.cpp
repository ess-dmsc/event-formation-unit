// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Event serialisation unit tests.
///
//===----------------------------------------------------------------------===//

#include "../EventSerializer.h"
#include "../EventData.h"
#include "ev42_events_generated.h"
#include <chrono>
#include <common/Producer.h>
#include <gtest/gtest.h>
#include <limits>
#include <trompeloeil.hpp>

using trompeloeil::_;

using namespace std::chrono_literals;

class ProducerStandIn : public ProducerBase {
public:
  ProducerStandIn() = default;
  MAKE_MOCK2(produce, int(nonstd::span<const std::uint8_t>, std::int64_t),
             override);
};

class SerializerStandIn : public EventSerializer {
public:
  std::atomic_bool UseOtherCurrentTime{false};
  std::atomic<std::chrono::system_clock::time_point> UsedCurrentTime{
      std::chrono::system_clock::now()};

  SerializerStandIn(std::string SourceName, size_t BufferSize,
                    std::chrono::milliseconds TransmitTimeout,
                    ProducerBase *KafkaProducer, TimestampMode Mode)
      : EventSerializer(std::move(SourceName), BufferSize, TransmitTimeout,
                        KafkaProducer, Mode) {}
  using EventSerializer::EventQueue;
  using EventSerializer::ReferenceTimeQueue;
  std::chrono::system_clock::time_point getCurrentTime() const override {
    if (UseOtherCurrentTime) {
      return UsedCurrentTime;
    }
    return std::chrono::system_clock::now();
  }
};

class EventSerialisationIndependent : public ::testing::Test {
public:
  void SetUp() override { Producer = std::make_unique<ProducerStandIn>(); }
  void SetUpSerializer(int BufferSize) {
    Serializer = std::make_unique<SerializerStandIn>(
        "SomeName", BufferSize, 60ms,
        dynamic_cast<ProducerBase *>(Producer.get()),
        EventSerializer::TimestampMode::INDEPENDENT_EVENTS);
  }
  std::unique_ptr<ProducerBase> Producer;
  std::unique_ptr<SerializerStandIn> Serializer;
};

TEST_F(EventSerialisationIndependent, ProduceFlatbufferOnOneEvent) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .TIMES(1)
      .RETURN(0);
  SetUpSerializer(1);
  Serializer->addEvent(std::make_unique<EventData>());
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

bool checkFlatbuffer1(nonstd::span<const std::uint8_t> Buffer,
                      std::int64_t MessageTimestampMS) {
  auto Verifier = flatbuffers::Verifier(Buffer.data(), Buffer.size());
  if (not VerifyEventMessageBuffer(Verifier)) {
    return false;
  }
  auto EventMessage = GetEventMessage(Buffer.data());
  auto AdcDebugData = EventMessage->facility_specific_data_as_AdcPulseDebug();
  if (not AdcDebugData->Verify(Verifier)) {
    return false;
  }
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
  auto Verifier = flatbuffers::Verifier(Buffer.data(), Buffer.size());
  if (not VerifyEventMessageBuffer(Verifier)) {
    return false;
  }
  auto EventMessage = GetEventMessage(Buffer.data());
  auto AdcDebugData = EventMessage->facility_specific_data_as_AdcPulseDebug();
  if (not AdcDebugData->Verify(Verifier)) {
    return false;
  }
  if (EventMessage->source_name()->str() != "SomeName") {
    return false;
  }
  if (EventMessage->message_id() >= 2) {
    return false;
  }
  if (EventMessage->pulse_time() < 1000000 or
      EventMessage->pulse_time() > 1000000 + 1000 * 3) {
    return false;
  }
  if (EventMessage->time_of_flight()->size() != 2) {
    return false;
  }
  if (EventMessage->detector_id()->size() != 2) {
    return false;
  }
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

TEST_F(EventSerialisationIndependent, CheckFlatbufferContents) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer1(_1, _2))
      .TIMES(1)
      .RETURN(0);
  SetUpSerializer(1);
  Serializer->addEvent(std::unique_ptr<EventData>(
      new EventData{1000000, 2, 3, 4, 5, 6000000, 7000000}));
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

using namespace std::chrono_literals;

TEST_F(EventSerialisationIndependent, ProduceFlatbufferOnTimeout) {
  bool CallDone = false;
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer1(_1, _2))
      .TIMES(1)
      .RETURN(0)
      .LR_SIDE_EFFECT(CallDone = true;);
  SetUpSerializer(2);
  auto RefTime = std::chrono::system_clock::now();
  Serializer->UsedCurrentTime = RefTime;
  Serializer->UseOtherCurrentTime = true;
  Serializer->addEvent(std::unique_ptr<EventData>(
      new EventData{1000000, 2, 3, 4, 5, 6000000, 7000000}));
  while (Serializer->EventQueue.size_approx() > 0) {
    std::this_thread::sleep_for(10ms);
  }
  std::this_thread::sleep_for(10ms);
  EXPECT_FALSE(CallDone);
  Serializer->UsedCurrentTime = RefTime + 75ms;
  std::this_thread::sleep_for(10ms);
  EXPECT_TRUE(CallDone);
}

bool checkFlatbufferTimeOverflow(nonstd::span<const std::uint8_t> Buffer,
                                 std::uint64_t BaseTimestamp) {
  auto Verifier = flatbuffers::Verifier(Buffer.data(), Buffer.size());
  if (not VerifyEventMessageBuffer(Verifier)) {
    return false;
  }
  auto EventMessage = GetEventMessage(Buffer.data());
  auto AdcDebugData = EventMessage->facility_specific_data_as_AdcPulseDebug();
  if (not AdcDebugData->Verify(Verifier)) {
    return false;
  }
  static int NrOfCalls = 0;
  NrOfCalls++;
  if (NrOfCalls == 1) {
    return EventMessage->pulse_time() == BaseTimestamp;
  }
  return EventMessage->pulse_time() ==
         BaseTimestamp + std::numeric_limits<std::uint32_t>::max() + 100;
}

TEST_F(EventSerialisationIndependent, ProduceFlatbufferOnTimestampOverflow1) {
  std::uint64_t BaseTimestamp = 1000000;
  std::uint64_t SecondTimestamp =
      BaseTimestamp + std::numeric_limits<std::uint32_t>::max() + 100;
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbufferTimeOverflow(_1, BaseTimestamp))
      .TIMES(2)
      .RETURN(0);
  SetUpSerializer(3);
  Serializer->addEvent(std::unique_ptr<EventData>(new EventData{
      BaseTimestamp, 2, 3, 4, 5, BaseTimestamp + 1, BaseTimestamp + 2}));
  Serializer->addEvent(std::unique_ptr<EventData>(new EventData{
      SecondTimestamp, 2, 3, 4, 5, SecondTimestamp + 1, SecondTimestamp + 2}));
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

TEST_F(EventSerialisationIndependent, NoFlatbufferProduced) {
  bool CallDone = false;
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer1(_1, _2))
      .TIMES(1)
      .RETURN(0)
      .LR_SIDE_EFFECT(CallDone = true;);
  SetUpSerializer(2);
  auto RefTime = std::chrono::system_clock::now();
  Serializer->UsedCurrentTime = RefTime;
  Serializer->UseOtherCurrentTime = true;
  Serializer->addEvent(std::unique_ptr<EventData>(
      new EventData{1000000, 2, 3, 4, 5, 6000000, 7000000}));
  while (Serializer->EventQueue.size_approx() > 0) {
    std::this_thread::sleep_for(10ms);
  }
  std::this_thread::sleep_for(10ms);
  EXPECT_FALSE(CallDone);
  Serializer->UsedCurrentTime = RefTime + 25ms;
  std::this_thread::sleep_for(10ms);
  EXPECT_FALSE(CallDone);
  Serializer.reset();
}

TEST_F(EventSerialisationIndependent, ProduceTwoFlatbuffersWithFourEvents) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer2(_1, _2))
      .TIMES(2)
      .RETURN(0);
  SetUpSerializer(2);
  std::uint64_t BaseTimestamp = 1000000;
  for (int i = 0; i < 4; i++) {
    Serializer->addEvent(std::unique_ptr<EventData>(new EventData{
        BaseTimestamp + i * 1000, 2, 3, 4, 5, BaseTimestamp + i * 1000 + 1,
        BaseTimestamp + i * 1000 + 2}));
  }
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

TEST_F(EventSerialisationIndependent, ProduceThreeFlatbuffersOnFiveEvents) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .TIMES(3)
      .RETURN(0);
  SetUpSerializer(2);
  std::uint64_t BaseTimestamp = 1000000;
  for (int i = 0; i < 5; i++) {
    Serializer->addEvent(std::unique_ptr<EventData>(new EventData{
        BaseTimestamp + i * 1000, 2, 3, 4, 5, BaseTimestamp + i * 1000 + 1,
        BaseTimestamp + i * 1000 + 2}));
  }
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

bool checkFlatbuffer3(nonstd::span<const std::uint8_t> Buffer) {
  auto Verifier = flatbuffers::Verifier(Buffer.data(), Buffer.size());
  auto EventMessage = GetEventMessage(Buffer.data());
  auto AdcDebugData = EventMessage->facility_specific_data_as_AdcPulseDebug();
  if (not AdcDebugData->Verify(Verifier)) {
    return false;
  }
  return VerifyEventMessageBuffer(Verifier);
}

TEST_F(EventSerialisationIndependent, ProduceOneFlatbufferOnTwoEvents) {
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbuffer3(_1))
      .TIMES(1)
      .RETURN(0);
  SetUpSerializer(2);
  std::uint64_t BaseTimestamp = 1000000;
  for (int i = 0; i < 2; i++) {
    Serializer->addEvent(std::unique_ptr<EventData>(new EventData{
        BaseTimestamp + i * 1000, 2, 3, 4, 5, BaseTimestamp + i * 1000 + 1,
        BaseTimestamp + i * 1000 + 2}));
  }
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

class EventSerialisationReferenced : public ::testing::Test {
public:
  void SetUp() override { Producer = std::make_unique<ProducerStandIn>(); }
  void SetUpSerializer(int BufferSize) {
    Serializer = std::make_unique<SerializerStandIn>(
        "SomeName", BufferSize, 60ms,
        dynamic_cast<ProducerBase *>(Producer.get()),
        EventSerializer::TimestampMode::TIME_REFERENCED);
  }
  std::unique_ptr<ProducerBase> Producer;
  std::unique_ptr<SerializerStandIn> Serializer;
};

TEST_F(EventSerialisationReferenced, ProduceFlatbufferOneEventNoRef) {
  std::uint64_t BaseTimestamp = 1000000;
  auto checkFlatbufferTimestamp = [BaseTimestamp](auto Buffer) {
    auto EventMessage = GetEventMessage(Buffer.data());
    if (EventMessage->pulse_time() != BaseTimestamp) {
      return false;
    }
    if (EventMessage->time_of_flight()->size() != 1) {
      return false;
    }
    if (EventMessage->time_of_flight()->operator[](0) != 0) {
      return false;
    }
    return true;
  };
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbufferTimestamp(_1))
      .TIMES(1)
      .RETURN(0);
  SetUpSerializer(1);
  Serializer->addEvent(std::unique_ptr<EventData>(new EventData{
      BaseTimestamp, 2, 3, 4, 5, BaseTimestamp + 1, BaseTimestamp + 2}));
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

TEST_F(EventSerialisationReferenced, ProduceFlatbufferTwoEventsNoRef) {
  std::uint64_t BaseTimestamp = 1000000;
  auto checkFlatbufferTimestamp = [BaseTimestamp](auto Buffer) {
    auto EventMessage = GetEventMessage(Buffer.data());
    if (EventMessage->pulse_time() != BaseTimestamp) {
      return false;
    }
    if (EventMessage->time_of_flight()->size() != 2) {
      return false;
    }
    if (EventMessage->time_of_flight()->operator[](0) != 0) {
      return false;
    }
    if (EventMessage->time_of_flight()->operator[](1) != 1000) {
      return false;
    }
    return true;
  };
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbufferTimestamp(_1))
      .TIMES(1)
      .RETURN(0);
  SetUpSerializer(2);
  for (int i = 0; i < 2; i++) {
    Serializer->addEvent(std::unique_ptr<EventData>(new EventData{
        BaseTimestamp + i * 1000, 2, 3, 4, 5, BaseTimestamp + i * 1000 + 1,
        BaseTimestamp + i * 1000 + 2}));
  }
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

TEST_F(EventSerialisationReferenced, ProduceFlatbufferOneEventOneRef) {
  std::uint64_t BaseTimestamp = 1000000;
  auto checkFlatbufferTimestamp = [BaseTimestamp](auto Buffer) {
    auto Verifier = flatbuffers::Verifier(Buffer.data(), Buffer.size());
    if (not VerifyEventMessageBuffer(Verifier)) {
      return false;
    }
    auto EventMessage = GetEventMessage(Buffer.data());
    auto AdcDebugData = EventMessage->facility_specific_data_as_AdcPulseDebug();
    if (not AdcDebugData->Verify(Verifier)) {
      return false;
    }
    if (EventMessage->pulse_time() != BaseTimestamp) {
      return false;
    }
    if (EventMessage->time_of_flight()->operator[](0) != 1000) {
      return false;
    }
    return true;
  };
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbufferTimestamp(_1))
      .TIMES(1)
      .RETURN(0);
  SetUpSerializer(1);
  Serializer->addReferenceTimestamp(BaseTimestamp);
  Serializer->addEvent(std::unique_ptr<EventData>(
      new EventData{BaseTimestamp + 1000, 2, 3, 4, 5, BaseTimestamp + 1000 + 1,
                    BaseTimestamp + 1000 + 2}));
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}

TEST_F(EventSerialisationReferenced, ProduceFlatbufferOneEventTwoRefs) {
  std::uint64_t BaseTimestamp = 1000000;
  std::uint64_t TestOffsetValue =
      std::numeric_limits<std::uint32_t>::max() + 100ull;
  auto TimesCalled{0};
  auto checkFlatbufferTimestamp = [BaseTimestamp, &TimesCalled,
                                   TestOffsetValue](auto Buffer) {
    ++TimesCalled;
    auto Verifier = flatbuffers::Verifier(Buffer.data(), Buffer.size());
    if (not VerifyEventMessageBuffer(Verifier)) {
      return false;
    }
    auto EventMessage = GetEventMessage(Buffer.data());
    auto AdcDebugData = EventMessage->facility_specific_data_as_AdcPulseDebug();
    if (not AdcDebugData->Verify(Verifier)) {
      return false;
    }
    if (TimesCalled == 1 and EventMessage->pulse_time() != BaseTimestamp) {
      return false;
    } else if (TimesCalled > 1 and
               EventMessage->pulse_time() != BaseTimestamp + TestOffsetValue) {
      return false;
    }
    if (EventMessage->time_of_flight()->operator[](0) != 1000) {
      return false;
    }
    return true;
  };
  REQUIRE_CALL(*dynamic_cast<ProducerStandIn *>(Producer.get()), produce(_, _))
      .WITH(checkFlatbufferTimestamp(_1))
      .TIMES(2)
      .RETURN(0);
  SetUpSerializer(1);
  Serializer->addReferenceTimestamp(BaseTimestamp);
  Serializer->addReferenceTimestamp(BaseTimestamp + TestOffsetValue);
  Serializer->addEvent(std::unique_ptr<EventData>(
      new EventData{BaseTimestamp + 1000, 2, 3, 4, 5, BaseTimestamp + 1000 + 1,
                    BaseTimestamp + 1000 + 2}));
  Serializer->addEvent(std::unique_ptr<EventData>(
      new EventData{BaseTimestamp + 1000 + TestOffsetValue, 2, 3, 4, 5,
                    BaseTimestamp + 1000 + 1, BaseTimestamp + 1000 + 2}));
  do {
    std::this_thread::sleep_for(1ms);
  } while (Serializer->EventQueue.size_approx() > 0 or Serializer->ReferenceTimeQueue.size_approx() > 0);
  Serializer.reset();
}
