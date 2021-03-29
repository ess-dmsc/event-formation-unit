// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/EV42Serializer.h>
#include <dream/Counters.h>

namespace Jalousie {

struct DreamSettings {
  //std::string ConfigFile;
  //
  //
  //
};

using namespace memory_sequential_consistent; // Lock free fifo

class DreamBase : public Detector {
public:
  explicit DreamBase(BaseSettings const &Settings, struct DreamSettings &LocalDreamSettings);
  ~DreamBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  DreamSettings DreamModuleSettings;
  EV42Serializer * Serializer;
};

}
