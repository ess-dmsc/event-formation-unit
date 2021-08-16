// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief MBCAEN detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Detector.h>
#include <freia/Counters.h>

namespace Freia {

struct FreiaSettings {
  std::string FilePrefix{""};
  std::string ConfigFile{""};
  uint32_t H5SplitTime{0}; // split files every N seconds (0 is inactive)
};



class FreiaBase : public Detector {
public:
  FreiaBase(BaseSettings const &settings, struct FreiaSettings &LocalFreiaSettings);
  ~FreiaBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters{};
  FreiaSettings FreiaSettings;
};

}
