// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief MBCAEN detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <freia/Counters.h>

namespace Freia {

struct FreiaSettings {
  std::string FilePrefix{""};
  std::string ConfigFile{""};
  std::string CalibFile{""};
};



class FreiaBase : public Detector {
public:
  FreiaBase(BaseSettings const &settings, struct FreiaSettings &LocalFreiaSettings);
  ~FreiaBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters{};
  FreiaSettings FreiaModuleSettings;
  EV42Serializer *Serializer;
};

}
