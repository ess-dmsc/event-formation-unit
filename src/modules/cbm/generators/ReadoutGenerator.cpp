// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
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
#include <chrono>
#include <common/debug/Trace.h>
#include <common/testutils/bitmaps/BitMaps.h>
#include <common/time/ESSTime.h>
#include <functional>
#include <generators/functiongenerators/DistributionGenerator.h>
#include <generators/functiongenerators/LinearGenerator.h>
#include <modules/cbm/generators/ReadoutGenerator.h>
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
  CbmGroup->add_option("--beamMask", cbmSettings.BeamMask,
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

  if (cbmSettings.monitorType == CbmType::EVENT_0D) {
    generateEvent0DData(dataPtr);
  } else if (cbmSettings.monitorType == CbmType::EVENT_2D) {
    if (cbmSettings.BeamMask) {
      generate2DBitmapData(dataPtr);
    } else {
      generate2DData(dataPtr);
    }
  } else if (cbmSettings.monitorType == CbmType::IBM) {
    generateIBMData(dataPtr);
  } else {
    throw std::runtime_error("Unsupported monitor type");
  }
}

// Generate data for 0D event monitor
void ReadoutGenerator::generateEvent0DData(uint8_t *dataPtr) {

  for (uint32_t Readout = 0; Readout < ReadoutsPerPacket; Readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = cbmSettings.FenId;
    auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTime();
    dataPkt->TimeHigh = readoutTimeHigh;
    dataPkt->TimeLow = readoutTimeLow;
    dataPkt->Type = cbmSettings.monitorType;
    dataPkt->Channel = cbmSettings.ChannelId;
    dataPkt->ADC = 12345;
    dataPkt->NPos = 0;

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

// Generate data for 2D event monitor
void ReadoutGenerator::generate2DData(uint8_t *dataPtr) {

  for (uint32_t Readout = 0; Readout < ReadoutsPerPacket; Readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = reinterpret_cast<Parser::CbmReadout *>(dataPtr);
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // Write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = cbmSettings.FenId;
    auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTime();
    dataPkt->TimeHigh = readoutTimeHigh;
    dataPkt->TimeLow = readoutTimeLow;
    dataPkt->Type = cbmSettings.monitorType;
    dataPkt->Channel = cbmSettings.ChannelId;
    dataPkt->ADC = 12345;
    // Capture position at present two uniform distributions are used.
    dataPkt->Pos.XPos = PositionDist(RandomGenerator, 
      DistParamType(0, cbmSettings.MaxXValue));
    dataPkt->Pos.YPos = PositionDist(RandomGenerator,
      DistParamType(0, cbmSettings.MaxYValue));
    if (cbmSettings.Randomise) {
      // When noise is enabled the generator will reduce number of
      // neutrons at the start and end of (x, y) dimension.
      int xPos = NoiseDist(RandomGenerator);
      int yPos = NoiseDist(RandomGenerator);
      if (xPos % 2 == 0) {
        dataPkt->Pos.XPos += xPos;
        dataPkt->Pos.YPos += yPos;
      } else {
        dataPkt->Pos.XPos -= xPos;
        dataPkt->Pos.YPos -= yPos;
      }
    } 

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

// Generate data for 2D event monitor
void ReadoutGenerator::generate2DBitmapData(uint8_t *dataPtr) {

  // Create image grid with grid cell coordinates stored in a vector. If vector is populated
  // it indicates that grid layout have been made and this part can be skipped. 
  if (GridCoordinateVector.size() == 0) {
    if (IMAGE_SIZE.first > cbmSettings.MaxXValue) {
      throw std::runtime_error("Minimum Beam Mask width of 32 is larger than MaxXValue."
                               "MaxXValue must be 32 or larger.");
    }
    if (IMAGE_SIZE.second > cbmSettings.MaxYValue) {
      throw std::runtime_error("Minimum Beam Mask height of 32 is larger than MaxYValue"
                               "MaxYValue must be 32 or larger.");
    }

    //Calculate the a grid for all figures.
    const uint16_t gridX = cbmSettings.MaxXValue / IMAGE_SIZE.first;
    const uint16_t gridY = cbmSettings.MaxYValue / IMAGE_SIZE.second;
    // calculate a margin to put the grid in the center
    const uint16_t residualX = (cbmSettings.MaxXValue - gridX * IMAGE_SIZE.first) / 2;
    const uint16_t residualY = (cbmSettings.MaxYValue - gridY * IMAGE_SIZE.second) / 2;
    //Create the grid coordinate vector containing the CBM coordinate of 
    //upper left grid corner of each grid cell on a centred mask
    for (uint16_t xLoop = 0; xLoop < gridX; xLoop++) {
      for (uint16_t yLoop = 0; yLoop < gridY; yLoop++) {
        uint16_t xPos = residualX + xLoop * IMAGE_SIZE.first;
        uint16_t yPos = residualY + yLoop * IMAGE_SIZE.second;
        GridCoordinateVector.emplace_back(std::make_pair(xPos, yPos));
      }
    }
  }

  for (uint32_t Readout = 0; Readout < ReadoutsPerPacket; Readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = reinterpret_cast<Parser::CbmReadout *>(dataPtr);
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // Write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = cbmSettings.FenId;
    auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTime();
    dataPkt->TimeHigh = readoutTimeHigh;
    dataPkt->TimeLow = readoutTimeLow;
    dataPkt->Type = cbmSettings.monitorType;
    dataPkt->Channel = cbmSettings.ChannelId;
    dataPkt->ADC = 12345;

    //Calculate a xpos and ypos for the read out.
    //Find a grid cell for a readout and determine which figure is used
    const int gridIndex = Fuzzer.randomInterval(0, GridCoordinateVector.size());
    const int imageIndex = gridIndex % mImages.size();
    const Image &image = *mImages[imageIndex];
    //Now we have the figure lets find the pixel that has been read.
    //Image is saved as an array with pixel coordinates. To get a random pixel a random 
    //item in the array is read. When getting the pixel coordinates "image.size() - 1" 
    //is used. This will leave out the last pixel in the figure and create an asymmetric figure
    const int index = Fuzzer.randomInterval(1, image.size() - 1);
    const auto &[gridX, gridY] = image[index];
    // Now we have a x, y coordinate within the grid. 
    // Time to get the offset coordinate of current grid
    const auto &[offsetX, offsetY] = GridCoordinateVector[gridIndex];

    // Lets calculate the actual coordinate within the beam.
    dataPkt->Pos.XPos = gridX + offsetX;
    dataPkt->Pos.YPos = gridY + offsetY;

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

// Generate data for IBM type beam monitors
void ReadoutGenerator::generateIBMData(uint8_t *dataPtr) {

  if (cbmSettings.ShakeBeam) {
    // Use the pre-initialized RandomGenerator and ShakeBeamDist to generate
    // a random drift value for the whole pulse, which will be applied by
    // the function generator.
    RandomTimeDriftNS = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::microseconds(BeamShakeDistMs(RandomGenerator)));
  }

  for (uint32_t Readout = 0; Readout < ReadoutsPerPacket; Readout++) {

    if (numberOfReadouts < cbmSettings.NumReadouts) {

      // Get pointer to the data buffer and clear memory with zeros
      auto dataPkt = (Parser::CbmReadout *)dataPtr;
      memset(dataPkt, 0, sizeof(Parser::CbmReadout));

      // write data packet to the buffer
      dataPkt->FiberId = CBM_FIBER_ID;
      dataPkt->FENId = cbmSettings.FenId;
      dataPkt->DataLength = sizeof(Parser::CbmReadout);
      auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTime();
      dataPkt->TimeHigh = readoutTimeHigh;
      dataPkt->TimeLow = readoutTimeLow;
      dataPkt->Type = CbmType::IBM;

      // Currently we generating for 1 beam monitor only
      dataPkt->Channel = cbmSettings.ChannelId;
      dataPkt->ADC = 0;

      if (cbmSettings.generatorType == GeneratorType::Distribution) {
        distributionValueGenerator(dataPkt);
      } else if (cbmSettings.generatorType == GeneratorType::Linear) {
        linearValueGenerator(dataPkt);
      } else {
        fixedValueGenerator(dataPkt);
      }

      // Move pointer to next readout
      dataPtr += sizeof(Parser::CbmReadout);
      numberOfReadouts++;
    }
  }
}

void ReadoutGenerator::distributionValueGenerator(Parser::CbmReadout *value) {
  if (Generator == nullptr) {

    auto GenMaX = MILLISEC / Settings.Frequency;

    if (cbmSettings.ShakeBeam) {
      // Add the maximum shake beam time to the generator max value to ensure
      // that the generator can generate values for the whole shake beam range.
      // Generator max value is in milliseconds, shake beam range is in
      // microseconds.
      GenMaX += round(static_cast<float>(SHAKE_BEAM_US.second) / 1e3);
    }

    Generator = std::make_unique<DistributionGenerator>(
        static_cast<double>(GenMaX), cbmSettings.NumberOfBins);
  }

  auto readoutTime = esstime::ESSTime(value->TimeHigh, value->TimeLow);
  auto TofMs = esstime::nsToMilliseconds(getTimeOfFlightNS(readoutTime)).count();

  int Noise{0};

  // Add noise to the distribution value if enabled
  if (cbmSettings.Randomise) {
    Noise = NoiseDist(RandomGenerator);
  }

  value->NPos = 1000 * Generator->getValueByPos(TofMs) + Noise;
}

void ReadoutGenerator::linearValueGenerator(Parser::CbmReadout *value) {
  if (Generator == nullptr) {
    Generator = std::make_unique<LinearGenerator>(
        Settings.Frequency, cbmSettings.NumReadouts,
        cbmSettings.Gradient.value());
  }

  auto readoutTime = esstime::ESSTime(value->TimeHigh, value->TimeLow);
  auto TofMs = esstime::nsToMilliseconds(getTimeOfFlightNS(readoutTime)).count();

  value->NPos = Generator->getValueByPos(TofMs);
}

void ReadoutGenerator::fixedValueGenerator(Parser::CbmReadout *value) {
  value->NPos = cbmSettings.Value.value() + cbmSettings.Offset;
}

} // namespace cbm
// GCOVR_EXCL_STOP
