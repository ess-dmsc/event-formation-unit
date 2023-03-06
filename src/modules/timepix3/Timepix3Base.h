// Copyright (C) 2019 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Timepix3 detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <timepix3/Counters.h>
#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>

namespace Timepix3 {

class Timepix3Base : public Detector {

public:
  Timepix3Base(BaseSettings const &Settings);
  ~Timepix3Base() = default;

  void processingThread();

protected:
  struct Counters Counters;
  EV44Serializer *Serializer;
};

} // namespace Timepix3
