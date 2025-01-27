// Copyright (C) 2024-2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Serialization objects for DA00 schema da00_DataArray and
/// da00_Variable tables
///
/// \author Gregory Tucker \date 2024-03-01
/// \link https://github.com/g5t/flatbuffer-histogram-generator
///
/// For flatbuffers schema see:
/// \link https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#include <common/kafka/serializer/AbstractSerializer.h>
#include <common/time/ESSTime.h>

namespace fbserializer {

AbstractSerializer::AbstractSerializer(const ProducerCallback Callback,
                                       SerializerStats &Stats)
    : ProduceCallback(std::move(Callback)), Stats(Stats) {}
flatbuffers::DetachedBuffer Buffer;

void AbstractSerializer::produce() {
  Stats.ProduceCalled++;

  if (!ReferenceTime.has_value()) {
    Stats.ProduceFailedNoReferenceTime++;
    return;
  }

  uint64_t CurrentHwClock =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch())
          .count();

  serialize();

  auto DataBuffer = nonstd::span<const uint8_t>(Buffer.data(), Buffer.size());

  ProduceCallback(DataBuffer, CurrentHwClock);
}

void AbstractSerializer::checkAndSetReferenceTime(
    const TimeDurationNano &Time) {

  // Produce already collected data before change reference time
  if (!ReferenceTime.has_value()) {
    ReferenceTime = Time;
    return;

  } else if (Time == ReferenceTime.value()) {
    return;
  }

  // Update reference time
  produce();
  Stats.ProduceRefTimeTriggered++;
  ReferenceTime = Time;
}

} // namespace fbserializer
