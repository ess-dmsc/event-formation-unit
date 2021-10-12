// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <dream/Counters.h>

namespace Dream {

struct DreamSettings {
  std::string ConfigFile{""}; ///< instrument mapping
};

class DreamBase : public Detector {
public:
  explicit DreamBase(BaseSettings const &Settings,
                     struct DreamSettings &LocalDreamSettings);
  ~DreamBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  DreamSettings DreamModuleSettings;
  EV42Serializer *Serializer;
};

} // namespace Dream
