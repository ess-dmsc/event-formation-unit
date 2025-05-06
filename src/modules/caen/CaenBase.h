// Copyright (C) 2019 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Caen detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <caen/CaenCounters.h>
#include <common/detector/Detector.h>
#include <common/types/DetectorType.h>

namespace Caen {

class CaenBase : public Detector {
  DetectorType Type;

public:
  CaenBase(BaseSettings const &Settings, DetectorType type);
  ~CaenBase() = default;

  void processingThread();

  struct CaenCounters Counters;

protected:
  std::vector<std::shared_ptr<EV44Serializer>> Serializers;
};

} // namespace Caen
