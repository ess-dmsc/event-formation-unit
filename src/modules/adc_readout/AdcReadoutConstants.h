/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parsing code for ADC readout.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

/// \brief Clock frequency of the MRF timing hardware clock.
static const std::int32_t TimerClockFrequencyExternal = 88'052'500; // Hz

static const std::int32_t TimerClockFrequencyInternal = 90'000'000; // Hz
