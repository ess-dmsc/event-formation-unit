/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EV42Serializer.h>
#include "ev42_events_generated.h"
#include <common/gccintel.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

static constexpr size_t TimeSize = sizeof(uint32_t);
static constexpr size_t PixelSize = sizeof(uint32_t);

// These must be non-0 because of Flatbuffers being stupid
// If they are initially set to 0, they will not be mutable
static constexpr uint64_t FBMutablePlaceholder = 1;

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

EV42Serializer::EV42Serializer(size_t MaxArrayLength, std::string SourceName, ProducerCallback Callback)
    : MaxEvents(MaxArrayLength), Builder_(MaxEvents * 8 + 256), ProduceFunctor(Callback) {

  auto SourceNameOffset = Builder_.CreateString(SourceName);
  auto TimeOffset = Builder_.CreateUninitializedVector(MaxEvents, TimeSize, &TimePtr);
  auto PixelOffset = Builder_.CreateUninitializedVector(MaxEvents, PixelSize, &PixelPtr);

  auto HeaderOffset = CreateEventMessage(Builder_, SourceNameOffset,
      FBMutablePlaceholder, FBMutablePlaceholder, TimeOffset, PixelOffset);
  FinishEventMessageBuffer(Builder_, HeaderOffset);

  Buffer_ = nonstd::span<const uint8_t >(Builder_.GetBufferPointer(), Builder_.GetSize());

  EventMessage_ = const_cast<EventMessage *>(GetEventMessage(Builder_.GetBufferPointer()));
  TimeLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(EventMessage_->time_of_flight()->Data())) - 1;
  PixelLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(EventMessage_->detector_id()->Data())) - 1;

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
  if (EventCount != 0) {
    XTRACE(OUTPUT, DEB, "autoproduce %zu EventCount_ \n", EventCount);
    serialize();
    if (ProduceFunctor) {
      // pulse_time is currently ns since 1970, produce time should be ms.
      ProduceFunctor(Buffer_, EventMessage_->pulse_time()/1000000);
    }
    return Buffer_.size_bytes();
  }
  return 0;
}

void EV42Serializer::pulseTime(uint64_t Time) {
  EventMessage_->mutate_pulse_time(Time);
}

uint64_t EV42Serializer::pulseTime() const {
  return EventMessage_->pulse_time();
}

size_t EV42Serializer::eventCount() const {
  return EventCount;
}

uint64_t EV42Serializer::currentMessageId() const {
  return MessageId;
}

size_t EV42Serializer::addEvent(uint32_t Time, uint32_t Pixel) {
  XTRACE(OUTPUT, DEB, "Add event: %d %u\n", Time, Pixel);
  reinterpret_cast<uint32_t*>(TimePtr)[EventCount] = Time;
  reinterpret_cast<uint32_t*>(PixelPtr)[EventCount] = Pixel;
  EventCount++;

  if (EventCount >= MaxEvents) {
    return produce();
  }
  return 0;
}
