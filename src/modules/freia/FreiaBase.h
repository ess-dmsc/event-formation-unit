// Copyright (C) 2021 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/AR52Serializer.h>
#include <freia/Counters.h>

namespace Freia {

class FreiaBase : public Detector {
public:
  FreiaBase(BaseSettings const &settings);
  ~FreiaBase() = default;

  void processing_thread();

  struct Counters Counters {};

protected:
  EV44Serializer *Serializer;
  AR52Serializer *MonitorSerializer;
};

} // namespace Freia
