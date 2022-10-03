// Copyright (C) 2016-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ev42 schema serialiser
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#include "ev42_events_generated.h"
#include <common/kafka/EV42Serializer.h>
#include <common/system/gccintel.h>

#include <common/debug/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

static constexpr size_t TimeSize = sizeof(uint32_t);
static constexpr size_t PixelSize = sizeof(uint32_t);

// These must be non-0 because of Flatbuffers being stupid
// If they are initially set to 0, they will not be mutable
static constexpr uint64_t FBMutablePlaceholder = 1;

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

EV42Serializer::EV42Serializer(size_t MaxArrayLength, std::string SourceName,
                               ProducerCallback Callback)
    : MaxEvents(MaxArrayLength), Builder_(MaxEvents * 8 + 256),
      ProduceFunctor(Callback) {

  auto SourceNameOffset = Builder_.CreateString(SourceName);
  auto TimeOffset =
      Builder_.CreateUninitializedVector(MaxEvents, TimeSize, &TimePtr);
  auto PixelOffset =
      Builder_.CreateUninitializedVector(MaxEvents, PixelSize, &PixelPtr);

  auto HeaderOffset =
      CreateEventMessage(Builder_, SourceNameOffset, FBMutablePlaceholder,
                         FBMutablePlaceholder, TimeOffset, PixelOffset);
  FinishEventMessageBuffer(Builder_, HeaderOffset);

  Buffer_ = nonstd::span<const uint8_t>(Builder_.GetBufferPointer(),
                                        Builder_.GetSize());

  EventMessage_ =
      const_cast<EventMessage *>(GetEventMessage(Builder_.GetBufferPointer()));
  TimeLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(EventMessage_->time_of_flight()->Data())) -
      1;
  PixelLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(EventMessage_->detector_id()->Data())) -
      1;

  EventMessage_->mutate_message_id(0);
  EventMessage_->mutate_pulse_time(0);
}

void EV42Serializer::setProducerCallback(ProducerCallback Callback) {
  ProduceFunctor = Callback;
}

nonstd::span<const uint8_t> EV42Serializer::serialize() {
  if (EventCount > MaxEvents) {
    // \todo this should probably throw instead?
    return {};
  }
  EventMessage_->mutate_message_id(MessageId);
  *TimeLengthPtr = EventCount;
  *PixelLengthPtr = EventCount;

  // reset counter and increment message counter
  EventCount = 0;
  MessageId++;

  return Buffer_;
}

size_t EV42Serializer::produce() {
  // pulse_time is given as ns since 1970, produce time should be ms.
  uint64_t PulseTimeMs = EventMessage_->pulse_time()/1000000;

  if (EventCount != 0) {
    XTRACE(OUTPUT, INF, "produce %zu events, produce time (ms): %lu",
       EventCount, PulseTimeMs);
    serialize();
    if (ProduceFunctor) {
      ProduceFunctor(Buffer_, PulseTimeMs);
    }
    TxBytes += Buffer_.size_bytes();
    return Buffer_.size_bytes();
  }
  return 0;
}

void EV42Serializer::pulseTime(uint64_t Time) {
  EventMessage_->mutate_pulse_time(Time);
}

uint32_t EV42Serializer::checkAndSetPulseTime(uint64_t Time) {
  uint32_t bytesProduced = 0;
  if (Time != pulseTime()) {
    XTRACE(OUTPUT, INF, "Pulse time is new - force produce()", Time);
    bytesProduced = produce();
    pulseTime(Time);
  }
  return bytesProduced;
}

uint64_t EV42Serializer::pulseTime() const {
  return EventMessage_->pulse_time();
}

size_t EV42Serializer::eventCount() const { return EventCount; }

uint64_t EV42Serializer::currentMessageId() const { return MessageId; }

size_t EV42Serializer::addEvent(uint32_t Time, uint32_t Pixel) {
  XTRACE(OUTPUT, DEB, "Add event: %d %u\n", Time, Pixel);
  reinterpret_cast<uint32_t *>(TimePtr)[EventCount] = Time;
  reinterpret_cast<uint32_t *>(PixelPtr)[EventCount] = Pixel;
  EventCount++;

  if (EventCount >= MaxEvents) {
    XTRACE(OUTPUT, INF, "Buffer Full - force produce()");
    return produce();
  }
  return 0;
}
