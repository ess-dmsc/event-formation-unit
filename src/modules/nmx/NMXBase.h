// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <nmx/Counters.h>

namespace Nmx {

struct NMXSettings {
  std::string FilePrefix{""};
  std::string ConfigFile{""};
  std::string CalibFile{""};
  //
};

class NMXBase : public Detector {
public:
  NMXBase(BaseSettings const &settings,
            struct NMXSettings &LocalNMXSettings);
  ~NMXBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters {};
  NMXSettings NMXModuleSettings;
  EV42Serializer *Serializer;
};

} // namespace Nmx
