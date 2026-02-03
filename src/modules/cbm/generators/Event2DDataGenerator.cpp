// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of EVENT_2D data generator
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/generators/Event2DDataGenerator.h>
#include <modules/cbm/readout/Parser.h>

namespace cbm {

void Event2DDataGenerator::generateData(uint8_t *dataPtr,
                                      uint32_t readoutsPerPacket,
                                      esstime::ESSTime pulseTime) const {
  (void)pulseTime;  // Unused - Event2D doesn't use pulse time

  if (BeamMask) {
    generateBitmapData(dataPtr, readoutsPerPacket);
  } else {
    generateRegular2DData(dataPtr, readoutsPerPacket);
  }
}

void Event2DDataGenerator::generateRegular2DData(uint8_t *dataPtr,
                                                uint32_t readoutsPerPacket) const {
  for (uint32_t readout = 0; readout < readoutsPerPacket; readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = reinterpret_cast<Parser::CbmReadout *>(dataPtr);
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // Write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = FiberId;
    dataPkt->FENId = FenId;
    auto [readoutTimeHigh, readoutTimeLow] = ReadoutTimeGenerator();
    dataPkt->TimeHigh = readoutTimeHigh;
    dataPkt->TimeLow = readoutTimeLow;
    dataPkt->Type = CbmType::EVENT_2D;
    dataPkt->Channel = ChannelId;
    dataPkt->ADC = 12345;
    // Capture position using uniform distributions
    dataPkt->Pos.XPos = PositionDist(RandomGenerator,
                                     DistParamType(0, MaxXValue));
    dataPkt->Pos.YPos = PositionDist(RandomGenerator,
                                     DistParamType(0, MaxYValue));
    if (Randomise) {
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

void Event2DDataGenerator::initializeGridCoordinates() {
  if (GridCoordinateVector.size() == 0) {
    if (IMAGE_SIZE.first > MaxXValue) {
      throw std::runtime_error("Minimum Beam Mask width of 32 is larger than MaxXValue. "
                               "MaxXValue must be 32 or larger.");
    }
    if (IMAGE_SIZE.second > MaxYValue) {
      throw std::runtime_error("Minimum Beam Mask height of 32 is larger than MaxYValue. "
                               "MaxYValue must be 32 or larger.");
    }

    // Calculate the grid for all figures
    const uint16_t gridX = MaxXValue / IMAGE_SIZE.first;
    const uint16_t gridY = MaxYValue / IMAGE_SIZE.second;
    // Calculate margin to put the grid in the center
    const uint16_t residualX = (MaxXValue - gridX * IMAGE_SIZE.first) / 2;
    const uint16_t residualY = (MaxYValue - gridY * IMAGE_SIZE.second) / 2;

    // Create the grid coordinate vector containing the CBM coordinate of
    // upper left grid corner of each grid cell on a centred mask
    for (uint16_t xLoop = 0; xLoop < gridX; xLoop++) {
      for (uint16_t yLoop = 0; yLoop < gridY; yLoop++) {
        uint16_t xPos = residualX + xLoop * IMAGE_SIZE.first;
        uint16_t yPos = residualY + yLoop * IMAGE_SIZE.second;
        GridCoordinateVector.emplace_back(std::make_pair(xPos, yPos));
      }
    }
  }
}

void Event2DDataGenerator::generateBitmapData(uint8_t *dataPtr,
                                             uint32_t readoutsPerPacket) const {
  // Initialize grid coordinates if not already done
  // initializeGridCoordinates();

  if (mImages.empty()) {
    throw std::runtime_error("No bitmap images provided for mask generation");
  }

  for (uint32_t readout = 0; readout < readoutsPerPacket; readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = reinterpret_cast<Parser::CbmReadout *>(dataPtr);
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // Write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = FiberId;
    dataPkt->FENId = FenId;
    auto [readoutTimeHigh, readoutTimeLow] = ReadoutTimeGenerator();
    dataPkt->TimeHigh = readoutTimeHigh;
    dataPkt->TimeLow = readoutTimeLow;
    dataPkt->Type = CbmType::EVENT_2D;
    dataPkt->Channel = ChannelId;
    dataPkt->ADC = 12345;

    // Calculate xpos and ypos for the readout
    // Find a grid cell for a readout and determine which figure is used
    const int gridIndex = Fuzzer.randomInterval(0, GridCoordinateVector.size());
    const int imageIndex = gridIndex % mImages.size();
    const Image &image = *mImages[imageIndex];

    // Now we have the figure, find the pixel that has been read.
    // Image is saved as an array with pixel coordinates. To get a random pixel
    // a random item in the array is read.
    const int index = Fuzzer.randomInterval(1, image.size() - 1);
    const auto &[gridX, gridY] = image[index];

    // Now we have an x, y coordinate within the grid.
    // Get the offset coordinate of current grid
    const auto &[offsetX, offsetY] = GridCoordinateVector[gridIndex];

    // Calculate the actual coordinate within the beam
    dataPkt->Pos.XPos = gridX + offsetX;
    dataPkt->Pos.YPos = gridY + offsetY;

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

} // namespace cbm
// GCOVR_EXCL_STOP
