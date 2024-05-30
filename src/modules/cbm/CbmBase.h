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
#include <memory>
#include <modules/cbm/geometry/Config.h>

namespace cbm {

class CbmBase : public Detector {
public:
  CbmBase(BaseSettings const &settings);
  ~CbmBase() = default;

  void processing_thread();

  struct Counters Counters {};

protected:
  std::unique_ptr<EV44Serializer> EV44SerializerPtrs[Config::MaxFEN]
                                                    [Config::MaxChannel];
  std::unique_ptr<fbserializer::HistogramSerializer<int32_t>>
      HistogramSerializerPtrs[Config::MaxFEN][Config::MaxChannel];
};

} // namespace cbm
