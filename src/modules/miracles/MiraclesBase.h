// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Miracles detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <miracles/Counters.h>
#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>

namespace Miracles {

class MiraclesBase : public Detector {
public:
  MiraclesBase(BaseSettings const &Settings);
  ~MiraclesBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  EV42Serializer *Serializer;
};

} // namespace Miracles
