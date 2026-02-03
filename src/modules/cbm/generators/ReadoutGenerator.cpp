// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial readout data for CBM beam monitor types like
/// IBM, EVENT_0D, etc.
/// \todo add link
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cbm/CbmTypes.h>
#include <cbm/generators/DataGeneratorFactory.h>
#include <cbm/generators/ReadoutGenerator.h>
#include <chrono>
#include <common/debug/Trace.h>
#include <common/testutils/bitmaps/BitMaps.h>
#include <common/time/ESSTime.h>
#include <functional>
#include <string>
#include <thread>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

ReadoutGenerator::ReadoutGenerator() : ReadoutGeneratorBase(DetectorType::CBM) {

  // Set default values for the generator

  CLI::Validator genTypeValidator = CLI::Validator(
      [this](std::string &value) -> std::string {
        if (value == "Fixed" && !cbmSettings.Value.has_value()) {
          throw CLI::ValidationError("Requires --value to be set");
        } else if (value == "Linear" && !cbmSettings.Gradient.has_value()) {
          throw CLI::ValidationError("Requires --gradient to be set");
        }
        return "";
      },
      "");

  // clang-format off

  auto CbmGroup = app.add_option_group("CBM Options");
  CbmGroup->add_option("--monitor_type", cbmSettings.monitorType,
                       "Beam monitor type (" + CbmType::getTypeNames() + ")");
  CbmGroup->add_option("--fiber", cbmSettings.FiberId,
                       "Override Fiber ID (default 22)");
  CbmGroup->add_option("--fen", cbmSettings.FenId,
                       "Override FEN ID (default 0)");
  CbmGroup->add_option("--channel", cbmSettings.ChannelId,
                       "Override channel ID (default 0)");
  CbmGroup->add_option("--maxXValue", cbmSettings.MaxXValue,
                       "Maximum X coordinate value for 2D Beam monitor. "
                       "Valid numbers 0 - 65535 (default 512)");
  CbmGroup->add_option("--maxYValue", cbmSettings.MaxYValue,
                       "Maximum Y coordinate value for 2D Beam monitor. "
                       "Valid numbers 0 - 65535 (default 512)");
  CbmGroup->add_flag("--beamMask", cbmSettings.BeamMask,
                     "Generate readout with mask in front of beam. "
                     "Default (false)");

  auto IbmGroup = app.add_option_group("IBM Options");
  IbmGroup->add_option("--numReadouts", cbmSettings.NumReadouts,
                       "Number of readouts per pulse");
  IbmGroup->add_option("--value", cbmSettings.Value,
      "Fixed value for the value function (required for Fixed generator type)");
  IbmGroup->add_option("--gradient", cbmSettings.Gradient,
                      "Gradient of the Linear function. Units must be in nano seconds"
                      "(required for Linear generator type)");
  IbmGroup->add_option("--offset", cbmSettings.Offset,
                       "Function generator offset for the start value "
                       "(Optional for all generator type)");
  IbmGroup->add_option("--bins", cbmSettings.NumberOfBins,
      "Number of bins (sampling) of the distribution function (default 512)");

  IbmGroup->add_flag("--shake", cbmSettings.ShakeBeam,
                     "Use random drift value for each pulse to shake the beam "
                     "(default false)");
  IbmGroup->add_flag("--noise", cbmSettings.Randomise,
                     "Add noise to the distribution value (default false)");
  IbmGroup
      ->add_option("--generator_type", cbmSettings.generatorType,
                   "Set the generator type ([Dist, Fixed, Linear] default : Dist)")
      ->check(genTypeValidator);

  IbmGroup->add_option("--normFactor", cbmSettings.NormFactor,
                     "defines how many ADC measurements are summed by the hardware "
                     "(default 1)");

  // Load bitmaps. All bit maps are of size 32 * 32 
  // However they will be used as 40 * 40 then there is a margin
  // Around them
  mImages.push_back(&BitMaps::si1_0);
  mImages.push_back(&BitMaps::si1_1);
  mImages.push_back(&BitMaps::si2_0);
  mImages.push_back(&BitMaps::si2_1);
  mImages.push_back(&BitMaps::zeta);    
}
// clang-format on

void ReadoutGenerator::initialize(
    std::unique_ptr<FunctionGenerator> &&timeGenerator) {

  ReadoutGeneratorBase::initialize(std::move(timeGenerator));

  DataGenerator = DataGeneratorFactory::createDataGenerator(
      cbmSettings, Settings.Frequency,
      [this]() { return generateReadoutTime(); }, mImages);
}

void ReadoutGenerator::generateData() {

  auto dataPtr = (uint8_t *)Buffer;
  dataPtr += HeaderSize;

  DataGenerator->generateData(dataPtr, ReadoutsPerPacket, pulseTime);
}

} // namespace cbm
// GCOVR_EXCL_STOP
