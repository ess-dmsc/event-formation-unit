// Copyright (C) 2016 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/detector/Detector.h>
#include <dream/Counters.h>

namespace Dream {

class DreamBase : public Detector {
public:
  explicit DreamBase(BaseSettings const &Settings);
  ~DreamBase() = default;

  void processingThread();

  struct Counters Counters;

protected:
};

} // namespace Dream
