// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial readout data for BEER instrument
/// Event 2D data generation
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include "generators/essudpgen/ReadoutGeneratorBase.h"
#include <beer/generators/ReadoutGenerator.h>
#include <cbm/generators/Event2DDataGenerator.h>
#include <common/testutils/bitmaps/BitMaps.h>

namespace beer {

ReadoutGenerator::ReadoutGenerator()
    : ReadoutGeneratorBase(DetectorType::BEER) {

  // Set default values for the generator

  // clang-format off

  auto BeerGroup = app.add_option_group("BEER Options");
  BeerGroup->add_option("--fiber", BeerSettings.FiberId,
                       "Override Fiber ID (default 0)");
  BeerGroup->add_option("--fen", BeerSettings.FenId,
                       "Override FEN ID (default 0)");
  BeerGroup->add_option("--channel", BeerSettings.ChannelId,
                       "Override channel ID (default 0)");
  BeerGroup->add_option("--maxXValue", BeerSettings.MaxXValue,
                       "Maximum X coordinate value for BEER detector. "
                       "Valid numbers 0 - 65535 (default 512)");
  BeerGroup->add_option("--maxYValue", BeerSettings.MaxYValue,
                       "Maximum Y coordinate value for BEER detector. "
                       "Valid numbers 0 - 65535 (default 512)");
  BeerGroup->add_flag("--beamMask", BeerSettings.BeamMask,
                     "Generate readout with neutron mask pattern. "
                     "Default (false)");
  BeerGroup->add_flag("--noise", BeerSettings.Randomise,
                     "Randomise the data to add noise. Default (false)");

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

void ReadoutGenerator::generateData() {

  auto dataPtr = (uint8_t *)Buffer;
  dataPtr += HeaderSize;

  DataGenerator->generateData(dataPtr, ReadoutsPerPacket, pulseTime);
}

void ReadoutGenerator::initialize(
    std::unique_ptr<FunctionGenerator> &&timeGenerator) {

  // Call base class initialize
  ReadoutGeneratorBase::initialize(std::move(timeGenerator));

  // BEER generator only supports Event 2D data
  DataGenerator = std::make_unique<cbm::Event2DDataGenerator>(
      BeerSettings.FiberId, BeerSettings.FenId, BeerSettings.ChannelId,
      BeerSettings.MaxXValue, BeerSettings.MaxYValue, BeerSettings.BeamMask,
      BeerSettings.Randomise, [this]() { return generateReadoutTime(); },
      mImages);
}

} // namespace beer
// GCOVR_EXCL_STOP
