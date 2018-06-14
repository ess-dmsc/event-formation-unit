/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief ADC settings.
 */

#pragma once

#include <CLI11.hpp>

/// @brief ADC readout (and processing) specific settings.
struct AdcSettings {
  bool SerializeSamples{false};
  bool PeakDetection{false};
  bool SampleTimeStamp{false};
  int TakeMeanOfNrOfSamples{1};
  bool IncludeTimeStamp{false};
  std::string TimeStampLocation{"Middle"};
  std::string Name;
  std::string GrafanaNameSuffix;
};

void SetCLIArguments(CLI::App &parser, AdcSettings &ReadoutSettings);
