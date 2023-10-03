// Copyright (C) 2021 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <dream/Counters.h>

namespace Dream {

class DreamBase : public Detector {
public:
  explicit DreamBase(BaseSettings const &Settings);
  ~DreamBase() = default;

  void processingThread();

  struct Counters Counters;

protected:
  EV44Serializer *Serializer;
};

} // namespace Dream
