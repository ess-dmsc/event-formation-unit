// Copyright (C) 2019 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Caen detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <caen/Counters.h>

namespace Caen {

struct CaenSettings {
  std::string ConfigFile{""}; ///< panel mappings
  std::string CalibFile{""};  ///< calibration file
  std::string FilePrefix{""}; ///< HDF5 file dumping
};

class CaenBase : public Detector {
public:
  CaenBase(BaseSettings const &Settings,
           struct CaenSettings &LocalCaenSettings);
  ~CaenBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  CaenSettings CaenModuleSettings;
  EV44Serializer *Serializer;
  EV44Serializer *SerializerII;
};

} // namespace Caen
