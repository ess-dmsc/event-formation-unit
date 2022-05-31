// Copyright (C) 2016-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ev44 schema serialiser
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#include <common/kafka/EV44Serializer.h>
#include "ev44_events_generated.h"
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

EV44Serializer::EV44Serializer(size_t MaxArrayLength, std::string SourceName, ProducerCallback Callback)
    : MaxEvents(MaxArrayLength), Builder_(MaxEvents * 8 + 256), ProduceFunctor(Callback) {

  auto SourceNameOffset = Builder_.CreateString(SourceName);
  auto TimeOffset = Builder_.CreateUninitializedVector(MaxEvents, TimeSize, &TimePtr);
  auto PixelOffset = Builder_.CreateUninitializedVector(MaxEvents, PixelSize, &PixelPtr);

  auto HeaderOffset = CreateEvent44Message(Builder_, SourceNameOffset,
      FBMutablePlaceholder, FBMutablePlaceholder, TimeOffset, PixelOffset);
  FinishEvent44MessageBuffer(Builder_, HeaderOffset);

  Buffer_ = nonstd::span<const uint8_t >(Builder_.GetBufferPointer(), Builder_.GetSize());

  Event44Message_ = const_cast<Event44Message *>(GetEvent44Message(Builder_.GetBufferPointer()));
  TimeLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(Event44Message_->time_of_flight()->Data())) - 1;
  PixelLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(Event44Message_->detector_id()->Data())) - 1;

  Event44Message_->mutate_message_id(0);
  Event44Message_->mutate_pulse_time(std::vector(0));
}

void EV44Serializer::setProducerCallback(ProducerCallback Callback) {
  ProduceFunctor = Callback;
}

nonstd::span<const uint8_t> EV44Serializer::serialize() {
  if (EventCount > MaxEvents) {
    // \todo this should probably throw instead?
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
  if (EventCount != 0) {
    XTRACE(OUTPUT, DEB, "autoproduce %zu EventCount_ \n", EventCount);
    serialize();
    if (ProduceFunctor) {
      // pulse_time is currently ns since 1970, produce time should be ms.
      ProduceFunctor(Buffer_, Event44Message_->pulse_time()/1000000);
    }
    return Buffer_.size_bytes();
  }
  return 0;
}

void EV44Serializer::referenceTime(uint64_t Time) {
  Event44Message_->mutate_reference_time(Time);
}

uint64_t EV44Serializer::referenceTime() const {
  return Event44Message_->reference_time();
}

size_t EV44Serializer::eventCount() const {
  return EventCount;
}

uint64_t EV44Serializer::currentMessageId() const {
  return MessageId;
}

size_t EV44Serializer::addEvent(uint32_t Time, uint32_t Pixel) {
  XTRACE(OUTPUT, DEB, "Add event: %d %u\n", Time, Pixel);
  reinterpret_cast<uint32_t*>(TimePtr)[EventCount] = Time;
  reinterpret_cast<uint32_t*>(PixelPtr)[EventCount] = Pixel;
  EventCount++;

  if (EventCount >= MaxEvents) {
    return produce();
  }
  return 0;
}
