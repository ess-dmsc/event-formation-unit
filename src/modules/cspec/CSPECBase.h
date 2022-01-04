// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <cspec/Counters.h>

namespace Cspec {

struct CSPECSettings {
  std::string FilePrefix{""};
  std::string ConfigFile{""};
  std::string CalibFile{""};
  //
};



class CSPECBase : public Detector {
public:
  CSPECBase(BaseSettings const &settings, struct CSPECSettings &LocalCSPECSettings);
  ~CSPECBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters{};
  CSPECSettings CSPECModuleSettings;
  EV42Serializer *Serializer;
};

}
