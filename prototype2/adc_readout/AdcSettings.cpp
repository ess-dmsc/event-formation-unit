/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Populate the command line parser with possible arguments.
 */

#include "AdcSettings.h"

void SetCLIArguments(CLI::App &parser, AdcSettingsStruct &AdcSettings) {
  parser
      .add_flag("--serialize_samples", AdcSettings.SerializeSamples,
                "Serialize sample data and send to Kafka broker.")
      ->group("ADC Readout Options");
  parser
      .add_flag("--peak_detection", AdcSettings.PeakDetection,
                "Find the maximum value in a range of samples and send that "
                "value along with its time-stamp to he Kafka broker.")
      ->group("ADC Readout Options");
  parser
      .add_option("--name", AdcSettings.Name,
                  "Name of the source of the data as made available on the "
                  "Kafka broker.")
      ->group("ADC Readout Options")
      ->set_default_val("AdcDemonstrator");
  parser
      .add_flag("--sample_timestamp", AdcSettings.SampleTimeStamp,
                "Provide a timestamp with every single ADC sample. Note: this "
                "drastically increases the bandwidth requirements.")
      ->group("ADC Readout Options");
  auto IsPositiveInt = [&AdcSettings](std::vector<std::string> Input) -> bool {
    int InputVal;
    try {
      InputVal = std::stoi(Input[0]);
      if (InputVal < 1) {
        return false;
      }
    } catch (std::invalid_argument &E) {
      return false;
    }
    AdcSettings.TakeMeanOfNrOfSamples = InputVal;
    return true;
  };
  CLI::callback_t CBFunc(IsPositiveInt);
  parser
      .add_option("--mean_of_samples", CBFunc,
                  "Only used when serializing data. Take the mean of # of "
                  "samples (oversample) and serialize that mean.")
      ->group("ADC Readout Options")
      ->set_default_val("1");
  parser
      .add_set("--time_stamp_loc", AdcSettings.TimeStampLocation,
               {"Start", "Middle", "End"},
               "Only used when serializing oversampled data. The time stamp "
               "corresponds to one of the following: 'Start', 'Middle', 'End'.")
      ->group("ADC Readout Options")
      ->set_default_val("Middle");
}
