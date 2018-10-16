/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Populate the command line parser with possible arguments.
 */

#include "AdcSettings.h"
#include "AdcReadoutConstants.h"

using PosType = AdcSettings::PositionSensingType;
std::istream &operator>>(std::istream &in, PosType &level);
std::ostream &operator<<(std::ostream &in, const PosType &level);

using ChRole = AdcSettings::ChannelRole;
std::istream &operator>>(std::istream &in, ChRole &level);
std::ostream &operator<<(std::ostream &in, const ChRole &level);
std::map<std::string, ChRole> getRoleMapping();

void SetCLIArguments(CLI::App &parser, AdcSettings &ReadoutSettings) {
  parser
      .add_flag("--serialize_samples", ReadoutSettings.SerializeSamples,
                "Serialize sample data and send to Kafka broker.")
      ->group("ADC Readout Options");
  parser
      .add_flag("--peak_detection", ReadoutSettings.PeakDetection,
                "Find the maximum value in a range of samples and send that "
                "value along with its time-stamp to he Kafka broker.")
      ->group("ADC Readout Options");
  parser
      .add_option("--name", ReadoutSettings.Name,
                  "Name of the source of the data as made available on the "
                  "Kafka broker.")
      ->group("ADC Readout Options")
      ->default_str("AdcDemonstrator");
  parser
      .add_option("--stats_suffix", ReadoutSettings.GrafanaNameSuffix,
                  "Grafana root name suffix, used for the stats.")
      ->group("ADC Readout Options")
      ->default_str("");
  parser
      .add_flag("--sample_timestamp", ReadoutSettings.SampleTimeStamp,
                "Provide a timestamp with every single ADC sample. Note: this "
                "drastically increases the bandwidth requirements.")
      ->group("Sampling Options");
  auto IsPositiveInt =
      [&ReadoutSettings](std::vector<std::string> Input) -> bool {
    int InputVal;
    try {
      InputVal = std::stoi(Input[0]);
      if (InputVal < 1 or InputVal > AdcTimerCounterMax) {
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
  parser
      .add_option("--mean_of_samples", CBFunc,
                  "Only used when serializing data. Take the mean of # of "
                  "samples (oversample) and serialize that mean.")
      ->group("Sampling Options")
      ->default_str("1");
  parser
      .add_set("--time_stamp_loc", ReadoutSettings.TimeStampLocation,
               {"Start", "Middle", "End"},
               "Only used when serializing oversampled data. The time stamp "
               "corresponds to one of the following: 'Start', 'Middle', 'End'.")
      ->group("Sampling Options")
      ->default_str("Middle");
  parser
      .add_option("--delayline_topic", ReadoutSettings.DelayLineKafkaTopic,
                  "The Kafka topic to which the delay line event data should be"
                  " transmitted. Ignored if delay line processing is not "
                  "enabled. If empty string, use the default setting.")
      ->group("Sampling Options");
  parser
      .add_option(
          "--alt_detector_interface", ReadoutSettings.AltDetectorInterface,
          "The interface (actualy IP address) to which the alternative (other) "
          "ADC readout box is connected. Ignored if \"--alt_detector_port=0\".")
      ->group("Delay Line Options")
      ->default_str("0.0.0.0");
  parser
      .add_option("--alt_detector_port", ReadoutSettings.AltDetectorPort,
                  "The UDP port to which the second (alternative) ADC readout "
                  "box sends its data. Disables the second ADC readout box if "
                  "set to 0.")
      ->group("Delay Line Options")
      ->default_str("0");
  parser
      .add_option("--xaxis_offset", ReadoutSettings.XAxisCalibOffset,
                  "The offset of the x-axis postion value.")
      ->group("Delay Line Options")
      ->default_str("0.0");
  parser
      .add_option("--xaxis_slope", ReadoutSettings.XAxisCalibSlope,
                  "The slope multiplier of the x-axis postion value.")
      ->group("Delay Line Options")
      ->default_str("1.0");
  parser
      .add_set("--xaxis_position_type", ReadoutSettings.XAxis,
               {PosType::AMPLITUDE, PosType::TIME, PosType::CONST},
               "How to calculate the x-axis position. If set to \"CONST\", use "
               "the value of \"xaxis_offset\".")
      ->group("Delay Line Options")
      ->default_str("CONST");

  parser
      .add_option("--yaxis_offset", ReadoutSettings.YAxisCalibOffset,
                  "The offset of the y-axis postion value.")
      ->group("Delay Line Options")
      ->default_str("0.0");
  parser
      .add_option("--yaxis_slope", ReadoutSettings.XAxisCalibSlope,
                  "The slope multiplier of the y-axis postion value.")
      ->group("Delay Line Options")
      ->default_str("1.0");
  parser
      .add_set("--yaxis_position_type", ReadoutSettings.YAxis,
               {PosType::AMPLITUDE, PosType::TIME, PosType::CONST},
               "How to calculate the y-axis position. If set to \"CONST\", use "
               "the value of \"yaxis_offset\".")
      ->group("Delay Line Options")
      ->default_str("CONST");
  
  parser
  .add_option("--event_timeout", ReadoutSettings.EventTimeoutNS,
           "The maximum amount of time between pulses before throwing away the event. Value is in nanoseconds (ns).")
  ->group("Delay Line Options")
  ->default_str("100");

  std::set<ChRole> RoleOptions;
  for (auto &Item : getRoleMapping()) {
    RoleOptions.emplace(Item.second);
  }

  parser
      .add_set("--adc1_ch1_role", ReadoutSettings.ADC1Channel1,
               std::move(RoleOptions), "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  parser
      .add_set("--adc1_ch2_role", ReadoutSettings.ADC1Channel2,
               std::move(RoleOptions), "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  parser
      .add_set("--adc1_ch3_role", ReadoutSettings.ADC1Channel3,
               std::move(RoleOptions), "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  parser
      .add_set("--adc1_ch4_role", ReadoutSettings.ADC1Channel4,
               std::move(RoleOptions), "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11

  parser
      .add_set("--adc2_ch1_role", ReadoutSettings.ADC2Channel1,
               std::move(RoleOptions), "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  parser
      .add_set("--adc2_ch2_role", ReadoutSettings.ADC2Channel2,
               std::move(RoleOptions), "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  parser
      .add_set("--adc2_ch3_role", ReadoutSettings.ADC2Channel3,
               std::move(RoleOptions), "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
  parser
      .add_set("--adc2_ch4_role", ReadoutSettings.ADC2Channel4,
               std::move(RoleOptions), "Set the role of an input-channel.")
      ->group("Delay Line Options")
      ->default_str("NONE"); // Use std::move to work around a bug in CLI11
}

std::map<std::string, PosType> getTypeMapping() {
  return {{"AMP", PosType::AMPLITUDE},
          {"AMPLITUDE", PosType::AMPLITUDE},
          {"CONST", PosType::CONST},
          {"TIME", PosType::TIME}};
}

std::istream &operator>>(std::istream &in, PosType &Type) {
  std::map<std::string, PosType> TypeMap = getTypeMapping();
  std::string InString;
  in >> InString;

  Type = TypeMap.at(InString);
  return in;
}

std::ostream &operator<<(std::ostream &in, const PosType &Type) {
  std::map<PosType, std::string> TypeMap;
  for (auto &Item : getTypeMapping()) {
    TypeMap[Item.second] = Item.first;
  }
  return in << TypeMap.at(Type);
}

std::istream &operator>>(std::istream &in, ChRole &Role) {
  std::map<std::string, ChRole> RoleMap = getRoleMapping();
  std::string InString;
  in >> InString;

  Role = RoleMap.at(InString);
  return in;
}

std::ostream &operator<<(std::ostream &in, const ChRole &Role) {
  std::map<ChRole, std::string> RoleMap;
  for (auto &Item : getRoleMapping()) {
    RoleMap[Item.second] = Item.first;
  }
  return in << RoleMap.at(Role);
}

std::map<std::string, ChRole> getRoleMapping() {
  return {{"REF_TIME", ChRole::REFERENCE_TIME},
          {"AMP_X_1", ChRole::AMPLITUDE_X_AXIS_1},
          {"AMP_X_2", ChRole::AMPLITUDE_X_AXIS_2},
          {"AMP_Y_1", ChRole::AMPLITUDE_Y_AXIS_1},
          {"AMP_Y_2", ChRole::AMPLITUDE_Y_AXIS_2},
          {"TIME_X_1", ChRole::TIME_X_AXIS_1},
          {"TIME_X_2", ChRole::TIME_X_AXIS_2},
          {"TIME_Y_1", ChRole::TIME_Y_AXIS_1},
          {"TIME_Y_2", ChRole::TIME_Y_AXIS_2},
          {"NONE", ChRole::NONE}};
}
