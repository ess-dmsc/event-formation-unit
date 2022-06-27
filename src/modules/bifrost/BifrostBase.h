// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Bifrost detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <bifrost/Counters.h>

namespace Bifrost {

struct BifrostSettings {
  std::string ConfigFile{""};  ///< panel mappings
  // std::string CalibFile{""};   ///< calibration file
  // std::string FilePrefix{""};  ///< HDF5 file dumping
};

class BifrostBase : public Detector {
public:
  BifrostBase(BaseSettings const &Settings, struct BifrostSettings &LocalBifrostSettings);
  ~BifrostBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  BifrostSettings BifrostModuleSettings;
  EV42Serializer *Serializer;
};

} // namespace Bifrost
