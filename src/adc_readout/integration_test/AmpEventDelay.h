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
  SampleRunGenerator AnodeGen;
  SampleRunGenerator XPosGen;
  SampleRunGenerator YPosGen;

  PoissonDelayData sgd;
};

class AmpEventDelay : public PoissonDelay {
public:
  AmpEventDelay(AmpEventDelayData &agd) : PoissonDelay(agd.sgd), data(agd) {}

  AmpEventDelayData data;

  void genSamplesAndEnqueueSend() override;
};
