// Copyright (C) 2022 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/AR51Serializer.h>
#include <nmx/Counters.h>

namespace Nmx {

class NmxBase : public Detector {
public:
  NmxBase(BaseSettings const &settings);
  ~NmxBase() = default;

  void processing_thread();

  struct Counters Counters {};

protected:
  EV44Serializer *Serializer;
  AR51Serializer *MonitorSerializer;
};

} // namespace Nmx
