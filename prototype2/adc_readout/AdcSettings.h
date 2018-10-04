/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ADC settings.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <CLI/CLI.hpp>

/// \brief ADC readout (and processing) specific settings.
struct AdcSettings {
  bool SerializeSamples{false};
  bool PeakDetection{false};
  bool DelayLineDetector{false};
  bool SampleTimeStamp{false};
  int TakeMeanOfNrOfSamples{1};
  bool IncludeTimeStamp{false};
  std::string SamplesKafkaTopic;
  std::string TimeStampLocation{"Middle"};
  std::string Name;
  std::string GrafanaNameSuffix;
  std::string   AltDetectorInterface = {"0.0.0.0"};
  std::uint16_t AltDetectorPort = {0};
  
  double XAxisCalibOffset = {0.0};
  double XAxisCalibSlope = {1.0};
  double YAxisCalibOffset = {0.0};
  double YAxisCalibSlope = {1.0};
  
  enum class PositionSensingType {
    AMPLITUDE,
    TIME,
    CONST,
  };
  
  PositionSensingType XAxis = {PositionSensingType::CONST};
  PositionSensingType YAxis = {PositionSensingType::CONST};
  
  enum class ChannelRole {
    REFERENCE_TIME,
    AMPLITUDE_X_AXIS_1,
    AMPLITUDE_X_AXIS_2,
    AMPLITUDE_Y_AXIS_1,
    AMPLITUDE_Y_AXIS_2,
    TIME_X_AXIS_1,
    TIME_X_AXIS_2,
    TIME_Y_AXIS_1,
    TIME_Y_AXIS_2,
    NONE
  };
  ChannelRole ADC1Channel1 = {ChannelRole::NONE};
  ChannelRole ADC1Channel2 = {ChannelRole::NONE};
  ChannelRole ADC1Channel3 = {ChannelRole::NONE};
  ChannelRole ADC1Channel4 = {ChannelRole::NONE};
  
  ChannelRole ADC2Channel1 = {ChannelRole::NONE};
  ChannelRole ADC2Channel2 = {ChannelRole::NONE};
  ChannelRole ADC2Channel3 = {ChannelRole::NONE};
  ChannelRole ADC2Channel4 = {ChannelRole::NONE};
};

void SetCLIArguments(CLI::App &parser, AdcSettings &ReadoutSettings);
