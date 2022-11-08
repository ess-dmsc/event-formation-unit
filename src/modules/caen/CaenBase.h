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
#include <common/kafka/EV42Serializer.h>

namespace Caen {

class CaenBase : public Detector {
public:
  CaenBase(BaseSettings const &Settings);
  ~CaenBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  EV42Serializer *Serializer;
  EV42Serializer *SerializerII;
};

} // namespace Caen
