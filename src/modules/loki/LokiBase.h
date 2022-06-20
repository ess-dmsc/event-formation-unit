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
#include <common/kafka/EV42Serializer.h>
#include <loki/Counters.h>

namespace Loki {

struct LokiSettings {
  std::string ConfigFile{""};  ///< panel mappings
  std::string CalibFile{""};   ///< calibration file
  std::string FilePrefix{""};  ///< HDF5 file dumping
  uint16_t MinStraw{0};        ///< debug \todo remove
  uint16_t MaxStraw{65535};    ///< debug \todo remove
};

class LokiBase : public Detector {
public:
  LokiBase(BaseSettings const &Settings, struct LokiSettings &LocalLokiSettings);
  ~LokiBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  LokiSettings LokiModuleSettings;
  EV42Serializer *Serializer;
  EV42Serializer *SerializerII;
};

} // namespace Loki
