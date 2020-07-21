/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ADC settings.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "OffsetTime.h"
#include <CLI/CLI.hpp>

/// \brief ADC readout (and processing) specific settings.
struct AdcSettings {
  AdcSettings() = default;
  bool SerializeSamples{false};
  bool PeakDetection{false};
  bool DelayLineDetector{false};
  bool SampleTimeStamp{false};
  bool DumpParserExceptionsCount{false};
  int TakeMeanOfNrOfSamples{1};
  OffsetTime::Offset TimeOffsetSetting{OffsetTime::Offset::NONE};
  std::chrono::system_clock::time_point ReferenceTime;
  std::string DelayLineKafkaTopic;
  std::string TimeStampLocation{"Middle"};
  std::string Name;

  double Threshold{0.1};

  double XAxisCalibOffset{0.0};
  double XAxisCalibSlope{1.0};
  double YAxisCalibOffset{0.0};
  double YAxisCalibSlope{1.0};
  unsigned int EventTimeoutNS;

  enum class PositionSensingType {
    AMPLITUDE,
    TIME,
    CONST,
  };

  PositionSensingType XAxis{PositionSensingType::CONST};
  PositionSensingType YAxis{PositionSensingType::CONST};

  // Only for DelayLine system
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
  ChannelRole ADC1Channel1{ChannelRole::NONE};
  ChannelRole ADC1Channel2{ChannelRole::NONE};
  ChannelRole ADC1Channel3{ChannelRole::NONE};
  ChannelRole ADC1Channel4{ChannelRole::NONE};
};

void setCLIArguments(CLI::App &Parser, AdcSettings &ReadoutSettings);
