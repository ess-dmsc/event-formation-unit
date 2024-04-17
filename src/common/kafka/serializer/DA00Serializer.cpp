// Copyright (C) 2023 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization into da00 schema
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#include "DA00Serializer.h"

namespace Serializer {

using namespace da00_faltbuffers;

template <typename T>
DA00Serializer<T>::DA00Serializer(ProducerCallback callback)
    : AbstractSerializer(callback), builder(Frame1DHistogramBuilder<T>()) {}

} // namespace Serializer