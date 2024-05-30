// Copyright (C) 2022 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include "common/kafka/serializer/DA00HistogramSerializer.h"
#include <cbm/Counters.h>
#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <cstdint>
#include <map>
#include <memory>
#include <modules/cbm/geometry/Config.h>
#include <utility>

namespace cbm {

template <typename T> class SerializerMap {
public:
  SerializerMap() = default;

  void add(int FEN, int Channel, std::unique_ptr<T> &value) {
    int index = FEN * MaxFen + Channel;
    Serializers[index] = std::move(value);
  }

  std::unique_ptr<T> &get(int FEN, int Channel) noexcept(false) {
    int index = FEN * MaxFen + Channel;
    return Serializers.at(index);
  }

  std::map<int, std::unique_ptr<T>> &getAllSerializers() { return Serializers; }

private:
  const int MaxFen = Config::MaxFEN;
  std::map<int, std::unique_ptr<T>> Serializers;
};

class CbmBase : public Detector {
public:
  CbmBase(BaseSettings const &settings);
  ~CbmBase() = default;

  void processing_thread();

  struct Counters Counters {};

protected:
  SerializerMap<EV44Serializer> EV44SerializerPtrs;
  SerializerMap<fbserializer::HistogramSerializer<int32_t>>
      HistogramSerializerPtrs;
};

} // namespace cbm
