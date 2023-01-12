// Copyright (C) 2019 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Caen detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <caen/Counters.h>
#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>

namespace Caen {

class CaenBase : public Detector {
  ESSReadout::Parser::DetectorType type;

public:
  CaenBase(BaseSettings const &Settings, ESSReadout::Parser::DetectorType t);
  ~CaenBase() = default;

  void processingThread();

protected:
  struct Counters Counters;
  EV44Serializer *Serializer;
  EV44Serializer *SerializerII;
};

} // namespace Caen
