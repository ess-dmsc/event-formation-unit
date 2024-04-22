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

#include "common/kafka/Producer.h"
#include "flatbuffers/flatbuffers.h"
#include <chrono>
#include <cstddef>

namespace serializer {
using namespace std::chrono;

class AbstractSerializer {

  ProducerCallback _produceCallback;

protected:
  AbstractSerializer(ProducerCallback callback) : _produceCallback(callback){};
  flatbuffers::DetachedBuffer _buffer;

  virtual void serialize() = 0;

public:
  virtual ~AbstractSerializer() = default;

  virtual void produce() {
    uint64_t currentHwClock =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch())
            .count();

    serialize();

    _produceCallback(
        nonstd::span<const uint8_t>(_buffer.data(), _buffer.size()),
        currentHwClock);
  };
};
} // namespace serializer