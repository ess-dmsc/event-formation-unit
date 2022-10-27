// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Bifrost detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <bifrost/Counters.h>
#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>

namespace Bifrost {

class BifrostBase : public Detector {
public:
  BifrostBase(BaseSettings const &Settings);
  ~BifrostBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  EV42Serializer *Serializer;
};

} // namespace Bifrost
