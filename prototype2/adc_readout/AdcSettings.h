/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief ADC settings.
 */

#pragma once

#include <CLI11.hpp>

struct AdcSettingsStruct {
  bool SerializeSamples{false};
  bool PeakDetection{false};
  int TakeMeanOfNrOfSamples{1};
  std::string TimeStampLocation;
};

void SetCLIArguments(CLI::App &parser, AdcSettingsStruct &AdcSettings);
