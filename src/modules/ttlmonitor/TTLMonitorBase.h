// Copyright (C) 2022 European Spallation Source, see LICENSE file
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
#include <ttlmonitor/Counters.h>

namespace TTLMonitor {

class TTLMonitorBase : public Detector {
public:
  TTLMonitorBase(BaseSettings const &settings);
  ~TTLMonitorBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters {};
  std::vector<EV44Serializer> Serializers;

};

} // namespace TTLMonitor
