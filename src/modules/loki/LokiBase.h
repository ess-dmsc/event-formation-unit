// Copyright (C) 2019 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief LoKI detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/EV42Serializer.h>
#include <loki/Counters.h>

namespace Loki {

struct LokiSettings {
  std::string ConfigFile{""};  ///< panel mappings
  std::string CalibFile{""};   ///< calibration file
  std::string FilePrefix{""};  ///< HDF5 file dumping
  bool DetectorImage2D{false}; ///< generate pixels for 2D detector (else 3D)
};

class LokiBase : public Detector {
public:
  LokiBase(BaseSettings const &Settings, struct LokiSettings &LocalLokiSettings);
  ~LokiBase() = default;

  void inputThread();
  void processingThread();

  /// \brief generate a Udder test image
  void testImageUdder();

protected:
  struct Counters Counters;
  LokiSettings LokiModuleSettings;
  EV42Serializer *Serializer;
};

} // namespace Loki
