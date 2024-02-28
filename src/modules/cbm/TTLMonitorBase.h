// Copyright (C) 2022 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TTLMonitor detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <memory>
#include <cbm/Counters.h>

namespace TTLMonitor {

class TTLMonitorBase : public Detector {
public:
  TTLMonitorBase(BaseSettings const &settings);
  ~TTLMonitorBase() = default;

  void processing_thread();

  struct Counters Counters {};

protected:
  std::vector<std::unique_ptr<EV44Serializer>> SerializersPtr;
};

} // namespace TTLMonitor
