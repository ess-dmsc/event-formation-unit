// Copyright (C) 2019 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Timepix3 detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include "geometry/Config.h"
#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <memory>
#include <common/testutils/TestBase.h>
#include <timepix3/Counters.h>

class Timepix3BaseTest; // For access to private members for testing

namespace Timepix3 {

class Timepix3Base : public Detector {

public:
  Timepix3Base(BaseSettings const &Settings);
  ~Timepix3Base() = default;

  void processingThread();

private:
  friend class ::Timepix3BaseTest;
  friend void writePacketToRxFIFO<Timepix3Base>(Timepix3Base &Base, std::vector<uint8_t> Packet);

  std::unique_ptr<EV44Serializer> Serializer;
  Config timepix3Configuration;
  struct Counters Counters;
};
} // namespace Timepix3
