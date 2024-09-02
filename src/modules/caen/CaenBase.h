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

namespace Caen {

class CaenBase : public Detector {
  ESSReadout::Parser::DetectorType type;

public:
  CaenBase(BaseSettings const &Settings, ESSReadout::Parser::DetectorType t);
  ~CaenBase() = default;

  void processingThread();

  struct CaenCounters Counters;

protected:
  std::vector<std::shared_ptr<EV44Serializer>> Serializers;
};

} // namespace Caen
