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
  uint8_t TypeSubtype{0x10};
  std::string FilePrefix{""};
};


class TTLMonitorBase : public Detector {
public:
  TTLMonitorBase(BaseSettings const &settings, struct TTLMonitorSettings &LocalTTLMonitorSettings);
  ~TTLMonitorBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters{};
  TTLMonitorSettings TTLMonitorModuleSettings;
  EV42Serializer *Serializer;
};

}
