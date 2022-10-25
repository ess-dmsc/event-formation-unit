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

struct MiraclesSettings {
  std::string ConfigFile{""}; ///< panel mappings
  // std::string CalibFile{""};   ///< calibration file
  std::string FilePrefix{""}; ///< HDF5 file dumping
};

class MiraclesBase : public Detector {
public:
  MiraclesBase(BaseSettings const &Settings,
              struct MiraclesSettings &LocalMiraclesSettings);
  ~MiraclesBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  MiraclesSettings MiraclesModuleSettings;
  EV42Serializer *Serializer;
};

} // namespace Miracles
