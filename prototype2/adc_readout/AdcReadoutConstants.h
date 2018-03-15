/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Parsing code for ADC readout.
 */

#pragma once

static const std::int32_t TimerClockFrequency = 88052500;
static const std::int32_t AdcSamplingRate = TimerClockFrequency / 2;
static const std::int32_t AdcTimerCounterMax = AdcSamplingRate;
