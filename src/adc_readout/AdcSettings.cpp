/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Populate the command line parser with possible arguments.
 */

#include "AdcSettings.h"
#include "AdcReadoutConstants.h"
#include <regex>

using PosType = AdcSettings::PositionSensingType;
using ChRole = AdcSettings::ChannelRole;

auto stringToTime(std::string const &TimeString) {
  using std::string_literals::operator""s;
  std::regex DateTimeRegex{
      R"rr(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z$)rr"};
  std::smatch Match;
  if (not std::regex_match(TimeString, Match, DateTimeRegex)) {
    throw std::runtime_error("The string \"" + TimeString +
                             "\" is not a valid date time string.");
  }
  auto GetValue = [](auto Match, auto MinValue, auto MaxValue,
                     auto TypeOfValue) {
    auto IntValue = std::atoi(Match.c_str());
    if (IntValue > MaxValue or IntValue < MinValue) {
      throw std::runtime_error("The value "s + std::to_string(MaxValue) +
                               " is not a valid "s + TypeOfValue + " value."s);
    }
    return IntValue;
  };
  std::tm t{};
  t.tm_year = GetValue(Match[1].str(), 1970, 3000, "year") - 1900;
  t.tm_mon = GetValue(Match[2].str(), 1, 12, "month") - 1;
  t.tm_mday = GetValue(Match[3].str(), 1, 31, "day");
  t.tm_hour = GetValue(Match[4].str(), 0, 23, "hour");
  t.tm_min = GetValue(Match[5].str(), 0, 59, "minute");
  t.tm_sec = GetValue(Match[6].str(), 0, 60, "second");
  return std::chrono::system_clock::from_time_t(timegm(&t));
}

void setCLIArguments(CLI::App &Parser, AdcSettings &ReadoutSettings) {
  Parser
      .add_flag("--serialize_samples", ReadoutSettings.SerializeSamples,
                "Serialize sample data and send to Kafka broker.")
      ->group("ADC Readout Options");
  Parser
      .add_flag("--peak_detection", ReadoutSettings.PeakDetection,
                "Find the maximum value in a range of samples and send that "
                "value along with its time-stamp to he Kafka broker.")
      ->group("ADC Readout Options");
  Parser
      .add_flag("--delayline_efu", ReadoutSettings.DelayLineDetector,
                "Enable event formation of delay line pulse data.")
      ->group("ADC Readout Options");
  Parser
      .add_option("--name", ReadoutSettings.Name,
                  "Name of the source of the data as made available on the "
                  "Kafka broker.")
      ->group("ADC Readout Options")
      ->default_str("AdcDemonstrator");

  auto ParseOffsetTimestamp =
      [&ReadoutSettings](std::vector<std::string> const &Input) -> bool {
    auto TestString = Input.at(0);
    std::transform(TestString.begin(), TestString.end(), TestString.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (TestString == "none") {
      ReadoutSettings.TimeOffsetSetting = OffsetTime::NONE;
      return true;
    } else if (TestString == "now") {
      ReadoutSettings.TimeOffsetSetting = OffsetTime::NOW;
      return true;
    } else {
      try {
        ReadoutSettings.ReferenceTime = stringToTime(TestString);
        return true;
      } catch (std::runtime_error &Error) {
        return false;
      }
    }
    return false;
  };
  CLI::callback_t CBOffsetTime(ParseOffsetTimestamp);
  Parser
      .add_option("--time_stamp_offset", CBOffsetTime,
                  "Offset the timestamp of the processed events. Takes one of "
                  "two options:"
                  "\n1. NONE (No timestamp offset.)"
                  "\n2. NOW (The first packet timestamp will be offset such "
                  "that it will be the current timestamp)."
                  "\n3. Reference data and time the following format "
                  "\"1980-07-21T02:51:04Z\".")
      ->group("ADC Readout Options")
      ->default_str("NONE");
  Parser
      .add_flag("--sample_timestamp", ReadoutSettings.SampleTimeStamp,
                "Provide a timestamp with every single ADC sample. Note: this "
                "drastically increases the bandwidth requirements.")
      ->group("Sampling Options");
  auto IsPositiveInt =
      [&ReadoutSettings](std::vector<std::string> Input) -> bool {
    int InputVal;
    try {
      InputVal = std::stoi(Input[0]);
      if (InputVal < 1 or InputVal > TimerClockFrequencyInternal / 2) {
        return false;
      }
    } catch (std::invalid_argument &E) {
      return false;
    } catch (std::out_of_range &E) {
      return false;
    }
    ReadoutSettings.TakeMeanOfNrOfSamples = InputVal;
    return true;
  };
  CLI::callback_t CBFunc(IsPositiveInt);
  Parser
      .add_option("--mean_of_samples", CBFunc,
                  "Only used when serializing data. Take the mean of # of "
                  "samples (oversample) and serialize that mean.")
      ->group("Sampling Options")
      ->default_str("1");
  Parser
      .add_set("--time_stamp_loc", ReadoutSettings.TimeStampLocation,
               {"Start", "Middle", "End"},
               "Only used when serializing oversampled data. The time stamp "
               "corresponds to one of the following: 'Start', 'Middle', 'End'.")
      ->group("Sampling Options")
      ->default_str("Middle");
  Parser
      .add_option("--delayline_topic", ReadoutSettings.DelayLineKafkaTopic,
                  "The Kafka topic to which the delay line event data should be"
                  " transmitted. Ignored if delay line processing is not "
                  "enabled. If empty string, use the default setting.")
      ->group("Sampling Options")
      ->default_str("delayline_detector");
  Parser
      .add_option(
          "--alt_detector_interface", ReadoutSettings.AltDetectorInterface,
          "The interface (actualy IP address) to which the alternative (other) "
          "ADC readout box is connected. Ignored if \"--alt_detector_port=0\".")
      ->group("Delay Line Options")
      ->default_str("0.0.0.0");
  Parser
      .add_option("--alt_detector_port", ReadoutSettings.AltDetectorPort,
                  "The UDP port to which the second (alternative) ADC readout "
                  "box sends its data. Disables the second ADC readout box if "
                  "set to 0.")
      ->group("Delay Line Options")
      ->default_str("0");
  Parser
      .add_option("--xaxis_offset", ReadoutSettings.XAxisCalibOffset,
                  "The offset of the x-axis postion value.")
      ->group("Delay Line Options")
      ->default_str("0.0");
  Parser
      .add_option("--xaxis_slope", ReadoutSettings.XAxisCalibSlope,
                  "The slope multiplier of the x-axis postion value.")
      ->group("Delay Line Options")
      ->default_str("1.0");
  std::vector<std::pair<std::string, PosType>> PosTypeMap{
      {"AMP", PosType::AMPLITUDE},
      {"AMPLITUDE", PosType::AMPLITUDE},
      {"CONST", PosType::CONST},
      {"TIME", PosType::TIME}};
  Parser
      .add_option(
          "--xaxis_position_type", ReadoutSettings.XAxis,
          "How to calculate the x-axis position. If set to \"CONST\", use "
          "the value of \"xaxis_offset\".")
      ->transform(CLI::CheckedTransformer(PosTypeMap, CLI::ignore_case))
      ->group("Delay Line Options")
      ->default_val("CONST");

  Parser
      .add_option("--yaxis_offset", ReadoutSettings.YAxisCalibOffset,
                  "The offset of the y-axis postion value.")
      ->group("Delay Line Options")
      ->default_str("0.0");
  Parser
      .add_option("--yaxis_slope", ReadoutSettings.YAxisCalibSlope,
                  "The slope multiplier of the y-axis postion value.")
      ->group("Delay Line Options")
      ->default_str("1.0");
  Parser
      .add_option(
          "--yaxis_position_type", ReadoutSettings.YAxis,
          "How to calculate the y-axis position. If set to \"CONST\", use "
          "the value of \"yaxis_offset\".")
      ->transform(CLI::CheckedTransformer(PosTypeMap, CLI::ignore_case))
      ->group("Delay Line Options")
      ->default_val("CONST");

  Parser
      .add_option("--event_timeout", ReadoutSettings.EventTimeoutNS,
                  "The maximum amount of time between pulses before throwing "
                  "away the event. Value is in nanoseconds (ns).")
      ->group("Delay Line Options")
      ->default_str("100");

  Parser
      .add_option("--threshold", ReadoutSettings.Threshold,
                  "Set threshold timestamp to the sample where this value "
                  "(relative to the maximum value) is exceeded.")
      ->group("Delay Line Options")
      ->default_str("0.1");

  std::vector<std::pair<std::string, ChRole>> ChRoleMap{
      {"REF_TIME", ChRole::REFERENCE_TIME},
      {"AMP_X_1", ChRole::AMPLITUDE_X_AXIS_1},
      {"AMP_X_2", ChRole::AMPLITUDE_X_AXIS_2},
      {"AMP_Y_1", ChRole::AMPLITUDE_Y_AXIS_1},
      {"AMP_Y_2", ChRole::AMPLITUDE_Y_AXIS_2},
      {"TIME_X_1", ChRole::TIME_X_AXIS_1},
      {"TIME_X_2", ChRole::TIME_X_AXIS_2},
      {"TIME_Y_1", ChRole::TIME_Y_AXIS_1},
      {"TIME_Y_2", ChRole::TIME_Y_AXIS_2},
      {"NONE", ChRole::NONE}};
  Parser
      .add_option("--adc1_ch1_role", ReadoutSettings.ADC1Channel1,
                  "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->transform(CLI::CheckedTransformer(ChRoleMap, CLI::ignore_case))
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  Parser
      .add_option("--adc1_ch2_role", ReadoutSettings.ADC1Channel2,
                  "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->transform(CLI::CheckedTransformer(ChRoleMap, CLI::ignore_case))
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  Parser
      .add_option("--adc1_ch3_role", ReadoutSettings.ADC1Channel3,
                  "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->transform(CLI::CheckedTransformer(ChRoleMap, CLI::ignore_case))
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  Parser
      .add_option("--adc1_ch4_role", ReadoutSettings.ADC1Channel4,
                  "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->transform(CLI::CheckedTransformer(ChRoleMap, CLI::ignore_case))
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11

  Parser
      .add_option("--adc2_ch1_role", ReadoutSettings.ADC2Channel1,
                  "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->transform(CLI::CheckedTransformer(ChRoleMap, CLI::ignore_case))
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  Parser
      .add_option("--adc2_ch2_role", ReadoutSettings.ADC2Channel2,
                  "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->transform(CLI::CheckedTransformer(ChRoleMap, CLI::ignore_case))
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  Parser
      .add_option("--adc2_ch3_role", ReadoutSettings.ADC2Channel3,
                  "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->transform(CLI::CheckedTransformer(ChRoleMap, CLI::ignore_case))
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  Parser
      .add_option("--adc2_ch4_role", ReadoutSettings.ADC2Channel4,
                  "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->transform(CLI::CheckedTransformer(ChRoleMap, CLI::ignore_case))
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
}
