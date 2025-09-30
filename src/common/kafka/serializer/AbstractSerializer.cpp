// Copyright (C) 2024 - 2025 European Spallation Source, see LICENSE file
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
  // After Serialize a detached buffer have been created.
  // If detached buffer size is zero the previous pulse did not 
  // contain any relevant data.
  if (Buffer.size() == 0) {
    return;
  }

  auto DataBuffer = nonstd::span<const uint8_t>(Buffer.data(), Buffer.size());
  
  ProduceCallback(DataBuffer, CurrentHwClock);
}
} // namespace fbserializer
