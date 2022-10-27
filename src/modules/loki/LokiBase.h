// Copyright (C) 2019 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief LoKI detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <loki/Counters.h>

namespace Loki {

class LokiBase : public Detector {
public:
  LokiBase(BaseSettings const &Settings);
  ~LokiBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  EV44Serializer *Serializer;
  EV44Serializer *SerializerII;
};

} // namespace Loki
