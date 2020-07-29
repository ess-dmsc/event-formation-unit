/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Header file for calculating the delay of an event.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "PoissonDelay.h"

struct AmpEventDelayData {
  PoissonDelayData PoissonData;
  SampleRunGenerator AnodeGen;
  SampleRunGenerator XPosGen;
  SampleRunGenerator YPosGen;
};

class AmpEventDelay {
public:
  AmpEventDelayData data;

  std::chrono::duration<size_t, std::nano> calcDelaTime();
  void genSamplesAndQueueSend(const TimeStamp &Time);
};
