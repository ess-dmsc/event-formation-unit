// Copyright (C) 2019 - 2023 European Spallation Source, ERIC. See LICENSE file
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
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/AR51Serializer.h>

namespace Caen {

class CaenBase : public Detector {
  ESSReadout::Parser::DetectorType type;

public:
  CaenBase(BaseSettings const &Settings, ESSReadout::Parser::DetectorType t);
  ~CaenBase() = default;

  void processingThread();

  struct CaenCounters Counters;

protected:
  EV44Serializer *Serializer;
  EV44Serializer *SerializerII;
  AR51Serializer *MonitorSerializer;
};

} // namespace Caen
