// Copyright (C) 2022-2024 European Spallation Source, see LICENSE file
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

#pragma once

#include <chrono>
#include <common/kafka/Producer.h>
#include <flatbuffers/flatbuffers.h>

namespace serializer {
using namespace std::chrono;

struct SerializerStats {
  int64_t ProduceCalled{0};
};

class AbstractSerializer {

  const ProducerCallback &ProduceCallback;
  int64_t &ProduceCalled;

protected:
  AbstractSerializer(const ProducerCallback &Callback, SerializerStats &Stats)
      : ProduceCallback(Callback), ProduceCalled(Stats.ProduceCalled){};
  flatbuffers::DetachedBuffer Buffer;

  virtual void serialize() = 0;

public:
  virtual ~AbstractSerializer() = default;

  virtual void produce() {
    ProduceCalled++;
    uint64_t CurrentHwClock =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch())
            .count();

    serialize();

    ProduceCallback(nonstd::span<const uint8_t>(Buffer.data(), Buffer.size()),
                    CurrentHwClock);
  };
};
} // namespace serializer