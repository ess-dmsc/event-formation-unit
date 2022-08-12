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
#include <common/kafka/EV42Serializer.h>
#include <ttlmonitor/Counters.h>

namespace TTLMonitor {

struct TTLMonitorSettings {
  std::string ConfigFile{""};
  std::string FilePrefix{""};
  int ReduceEvents{1};
  int NumberOfMonitors{1};
};

class TTLMonitorBase : public Detector {
public:
  TTLMonitorBase(BaseSettings const &settings,
                 struct TTLMonitorSettings &LocalTTLMonitorSettings);
  ~TTLMonitorBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters {};
  TTLMonitorSettings TTLMonitorModuleSettings;
  std::vector<EV42Serializer> Serializers;
};

} // namespace TTLMonitor
