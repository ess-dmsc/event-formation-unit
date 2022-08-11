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


static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

EV44Serializer::EV44Serializer(size_t MaxArrayLength, std::string SourceName, ProducerCallback Callback)
    : MaxEvents(MaxArrayLength), Builder_(MaxEvents * 8 + 256), ProduceFunctor(Callback) {

  auto SourceNameOffset = Builder_.CreateString(SourceName);
  auto ReferenceTimeOffset = Builder_.CreateUninitializedVector(1, TimeSize, &ReferenceTimePtr);
  auto ReferenceTimeIndexOffset = Builder_.CreateVector<int32_t>(std::vector(1, 0));
  auto OffsetTimeOffset = Builder_.CreateUninitializedVector(MaxEvents, TimeSize, &OffsetTimePtr);
  auto PixelOffset = Builder_.CreateUninitializedVector(MaxEvents, PixelSize, &PixelPtr);

  auto HeaderOffset = CreateEvent44Message(Builder_, SourceNameOffset,
      ReferenceTimeOffset, ReferenceTimeIndexOffset, OffsetTimeOffset, PixelOffset);
  FinishEvent44MessageBuffer(Builder_, HeaderOffset);

  Buffer_ = nonstd::span<const uint8_t >(Builder_.GetBufferPointer(), Builder_.GetSize());

  Event44Message_ = const_cast<Event44Message *>(GetEvent44Message(Builder_.GetBufferPointer()));
  TimeLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(Event44Message_->time_of_flight()->Data())) - 1;
  PixelLengthPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(Event44Message_->pixel_id()->Data())) - 1;
}

void EV44Serializer::setProducerCallback(ProducerCallback Callback) {
  ProduceFunctor = Callback;
}

nonstd::span<const uint8_t> EV44Serializer::serialize() {
  if (EventCount > MaxEvents) {
    // \todo this should probably throw instead?
    return {};
  }
  *TimeLengthPtr = EventCount;
  *PixelLengthPtr = EventCount;

  // reset counter and increment message counter
  EventCount = 0;

  return Buffer_;
}

size_t EV44Serializer::produce() {
  ProduceTimer.reset();
  if (EventCount != 0) {
    XTRACE(OUTPUT, DEB, "autoproduce %zu EventCount_ \n", EventCount);
    serialize();
    if (ProduceFunctor) {
      // pulse_time is currently ns since 1970, produce time should be ms.
      ProduceFunctor(Buffer_, Event44Message_->reference_time()->data()[0]/1000000);
    }
    TxBytes += Buffer_.size_bytes();
    return Buffer_.size_bytes();
  }
  return 0;
}

size_t EV44Serializer::eventCount() const {
  return EventCount;
}


uint32_t EV44Serializer::checkAndSetReferenceTime(int64_t Time){
  uint32_t bytesProduced = 0;
  if (Time != referenceTime()){
     XTRACE(OUTPUT, DEB, "Reference time is new: %d\n", Time);
     bytesProduced = produce();
     setReferenceTime(Time);
  }
  return bytesProduced;
}

void EV44Serializer::setReferenceTime(int64_t Time){
  XTRACE(OUTPUT, DEB, "Set reference time: %d\n", Time);
  reinterpret_cast<int64_t*>(ReferenceTimePtr)[0] = Time;
}

int64_t EV44Serializer::referenceTime() const{
  return reinterpret_cast<int64_t*>(ReferenceTimePtr)[0];
}

size_t EV44Serializer::addEvent(int32_t Time, int32_t Pixel) {
  XTRACE(OUTPUT, DEB, "Add event: %d %u\n", Time, Pixel);
  reinterpret_cast<int32_t*>(OffsetTimePtr)[EventCount] = Time;  
  reinterpret_cast<int32_t*>(PixelPtr)[EventCount] = Pixel;
  EventCount++;

  if (EventCount >= MaxEvents) {
    XTRACE(DATA, DEB, "Serializer reached max events, producing message now");
    return produce();
  }
  return 0;
}
