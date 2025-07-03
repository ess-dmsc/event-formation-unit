// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/types/DetectorType.h>
#include <common/detector/Detector.h>
#include <dream/Counters.h>

namespace Dream {

class DreamBase : public Detector {
private:
  DetectorType Type;

public:
  explicit DreamBase(BaseSettings const &Settings, DetectorType Type);
  ~DreamBase() = default;

  void processingThread();

  struct Counters Counters;
};

} // namespace Dream
