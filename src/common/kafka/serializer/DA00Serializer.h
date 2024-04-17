// Copyright (C) 2023 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization into da00 schema
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#pragma once

#include "AbstractSerializer.h"
#include "builders/DA00FlatbufferBuilders.h"
#include "flatbuffers/flatbuffers.h"
#include <cstdint>

namespace Serializer {

using data_t = uint32_t;
using time_t = uint32_t;

template <typename T> class DA00Serializer : public AbstractSerializer {

public:
  DA00Serializer(ProducerCallback);

  size_t addEvent(const int32_t &rToA, const T &data);

  size_t produce() override;

public:
  da00_faltbuffers::Frame1DHistogramBuilder<T> &builder;

  int64_t SeqNum{0};
  int64_t TxBytes{0};
};

} // namespace Serializer