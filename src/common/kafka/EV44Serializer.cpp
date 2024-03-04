// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ev44 schema serialiser
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#include "ev44_events_generated.h"
#include <chrono>
#include <common/kafka/EV44Serializer.h>
#include <common/system/gccintel.h>

#include <common/debug/Trace.h>
#include <cstdint>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace std::chrono;

// defining the lengths of elements of the flatbuffer schema
//  eg. ReferenceTimeSize is 64 bits, the size of the reference_time elements
//  and OffsetTimeSize is 32 bits, the size of the reference_time_offset
//  elements
static constexpr size_t ReferenceTimeSize = sizeof(uint64_t);
static constexpr size_t OffsetTimeSize = sizeof(uint32_t);
static constexpr size_t PixelSize = sizeof(uint32_t);

// These must be non-0 because of Flatbuffers being stupid
// If they are initially set to 0, they will not be mutable
static constexpr int64_t FBMutablePlaceholder = 1;

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

EV44Serializer::EV44Serializer(size_t MaxArrayLength, std::string SourceName,
                               ProducerCallback Callback)
    : MaxEvents(MaxArrayLength), Builder_(MaxEvents * 8 + 256),
      ProduceFunctor(Callback) {

  auto SourceNameOffset = Builder_.CreateString(SourceName);
  auto ReferenceTimeOffset = Builder_.CreateUninitializedVector(
      1, ReferenceTimeSize, &ReferenceTimePtr);
  auto ReferenceTimeIndexOffset =
      Builder_.CreateVector<int32_t>(std::vector(1, 0));
  auto OffsetTimeOffset = Builder_.CreateUninitializedVector(
      MaxEvents, OffsetTimeSize, &OffsetTimePtr);
  auto PixelOffset =
      Builder_.CreateUninitializedVector(MaxEvents, PixelSize, &PixelPtr);

  auto HeaderOffset = CreateEvent44Message(
      Builder_, SourceNameOffset, FBMutablePlaceholder, ReferenceTimeOffset,
      ReferenceTimeIndexOffset, OffsetTimeOffset, PixelOffset);
  FinishEvent44MessageBuffer(Builder_, HeaderOffset);

  Buffer_ = nonstd::span<const uint8_t>(Builder_.GetBufferPointer(),
                                        Builder_.GetSize());

  Event44Message_ = const_cast<Event44Message *>(
      GetEvent44Message(Builder_.GetBufferPointer()));
  TimeLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(const_cast<std::uint8_t *>(
          Event44Message_->time_of_flight()->Data())) -
      1;
  PixelLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(Event44Message_->pixel_id()->Data())) -
      1;

  Event44Message_->mutate_message_id(0);

  ProduceCausePulseChange = 0;
  ProduceCauseMaxEventsReached = 0;
}

void EV44Serializer::setProducerCallback(ProducerCallback Callback) {
  ProduceFunctor = Callback;
}

nonstd::span<const uint8_t> EV44Serializer::serialize() {
  if (EventCount > MaxEvents) {
    /// \todo this should probably throw instead?
    return {};
  }
  Event44Message_->mutate_message_id(MessageId);
  *TimeLengthPtr = EventCount;
  *PixelLengthPtr = EventCount;

  // reset counter and increment message counter
  EventCount = 0;
  MessageId++;

  return Buffer_;
}

size_t EV44Serializer::produce() {
  ProduceTimer.reset();
  if (EventCount != 0) {
    XTRACE(OUTPUT, DEB, "autoproduce %zu EventCount_ \n", EventCount);
    serialize();
    if (ProduceFunctor) {
      
      // produce kafka message timestamp with current timestamp from harware
      // clock
      uint64_t currentHwClock =
          duration_cast<milliseconds>(system_clock::now().time_since_epoch())
              .count();
      ProduceFunctor(Buffer_, currentHwClock);
    }
    // \todo change to new producer metrics and remove
    TxBytes += Buffer_.size_bytes();
    return Buffer_.size_bytes();
  }
  return 0;
}

size_t EV44Serializer::eventCount() const { return EventCount; }

uint32_t EV44Serializer::checkAndSetReferenceTime(int64_t Time) {
  uint32_t bytesProduced = 0;
  if (Time != referenceTime()) {
    XTRACE(OUTPUT, DEB, "Reference time is new: %" PRIi64 "\n", Time);
    bytesProduced = produce();
    ProduceCausePulseChange++;
    setReferenceTime(Time);
  }
  return bytesProduced;
}

void EV44Serializer::setReferenceTime(int64_t Time) {
  XTRACE(OUTPUT, DEB, "Set reference time: %" PRIi64, Time);
  reinterpret_cast<int64_t *>(ReferenceTimePtr)[0] = Time;
}

int64_t EV44Serializer::referenceTime() const {
  return reinterpret_cast<int64_t *>(ReferenceTimePtr)[0];
}

uint64_t EV44Serializer::currentMessageId() const { return MessageId; }

size_t EV44Serializer::addEvent(int32_t Time, int32_t Pixel) {
  XTRACE(OUTPUT, DEB, "Add event: %d %u\n", Time, Pixel);
  reinterpret_cast<int32_t *>(OffsetTimePtr)[EventCount] = Time;
  reinterpret_cast<int32_t *>(PixelPtr)[EventCount] = Pixel;
  EventCount++;

  if (EventCount >= MaxEvents) {
    XTRACE(DATA, DEB, "Serializer reached max events, producing message now");
    ProduceCauseMaxEventsReached++;
    return produce();
  }
  return 0;
}
