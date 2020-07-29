/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcTimeStamp.h"
#include <functional>

#include "SampleRunGenerator.h"
#include "UdpConnection.h"

enum class SamplerType { PoissonDelay, AmpEventDelay, Continous };

struct SamplingTimerData {
  SamplerType Type;
  UdpConnection *UdpCon;
  SampleRunGenerator SampleGen;

  double Settings_offset;
  double Settings_amplitude;
  double Settings_rate;
};