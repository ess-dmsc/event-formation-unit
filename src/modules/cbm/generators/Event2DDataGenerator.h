// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial EVENT_2D readout data for CBM beam monitor
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/testutils/DataFuzzer.h>
#include <modules/cbm/generators/CbmDataGenerator.h>
#include <common/time/ESSTime.h>
#include <cstdint>
#include <functional>
#include <random>
#include <utility>
#include <vector>

namespace cbm {

///
/// \class Event2DDataGenerator
/// \brief Generator for EVENT_2D beam monitor readouts.
///
/// Generates 2D event data with random position information.
/// Supports both regular random distribution and bitmap mask modes.
///
class Event2DDataGenerator : public CbmDataGenerator {
public:
  /// \brief Constructor
  /// \param fiberId Fiber ID for readouts
  /// \param fenId FEN ID for readouts
  /// \param channelId Channel ID for readouts
  /// \param maxXValue Maximum X coordinate value
  /// \param maxYValue Maximum Y coordinate value
  /// \param beamMask Enable bitmap mask generation
  /// \param randomise Add noise to position values
  /// \param readoutTimeGenerator Function that returns (TimeHigh, TimeLow) tuple
  /// \param images Vector of bitmap images for mask (optional, used if beamMask is true)
  Event2DDataGenerator(
      uint8_t fiberId, uint8_t fenId, uint8_t channelId, uint16_t maxXValue,
      uint16_t maxYValue, bool beamMask, bool randomise,
      std::function<std::pair<uint32_t, uint32_t>()> readoutTimeGenerator,
      const std::vector<const std::vector<std::pair<uint8_t, uint8_t>> *>
          &images = {})
      : FiberId(fiberId), FenId(fenId), ChannelId(channelId),
        MaxXValue(maxXValue), MaxYValue(maxYValue), BeamMask(beamMask),
        Randomise(randomise), ReadoutTimeGenerator(readoutTimeGenerator),
        mImages(images) {

    if (BeamMask) {
      initializeGridCoordinates();
    } else {
      // Initialize position distribution only for regular 2D mode
      PositionDist.param(DistParamType(0, maxXValue));
    }
  }

  ///
  /// \brief Generate EVENT_2D readout data.
  ///
  /// \param dataPtr Pointer to the buffer where data should be written
  /// \param readoutsPerPacket Number of readouts to generate
  /// \param pulseTime Pulse time reference (optional, default is zero)
  ///
  void generateData(uint8_t *dataPtr, uint32_t readoutsPerPacket,
                    esstime::ESSTime pulseTime = esstime::ESSTime()) const override;

private:
  using DistParamType = std::uniform_int_distribution<>::param_type;
  using Coordinates = std::pair<uint8_t, uint8_t>;
  using Image = std::vector<Coordinates>;

  // Images size in pixels
  static constexpr Coordinates IMAGE_SIZE{32, 32};

  uint8_t FiberId;  ///< Fiber ID
  uint8_t FenId;    ///< FEN ID
  uint8_t ChannelId;             ///< Channel ID
  uint16_t MaxXValue;            ///< Maximum X coordinate
  uint16_t MaxYValue;            ///< Maximum Y coordinate
  bool BeamMask;                 ///< Generate with bitmap mask
  bool Randomise;                ///< Add noise to positions
  std::function<std::pair<uint32_t, uint32_t>()>
      ReadoutTimeGenerator;      ///< Function to generate readout times
  std::vector<const Image *> mImages;  ///< Bitmap images for mask
  std::vector<std::pair<uint16_t, uint16_t>>
      GridCoordinateVector;      ///< Grid coordinates for mask

  // Random generators for position
  mutable std::minstd_rand RandomGenerator{std::random_device{}()};
  mutable std::uniform_int_distribution<int> PositionDist{0, 512};
  mutable std::uniform_int_distribution<int> NoiseDist{0, 50};
  mutable DataFuzzer Fuzzer;

  ///
  /// \brief Generate regular 2D data with random positions.
  ///
  void generateRegular2DData(uint8_t *dataPtr, uint32_t readoutsPerPacket) const;

  ///
  /// \brief Generate 2D data with bitmap mask.
  ///
  void generateBitmapData(uint8_t *dataPtr, uint32_t readoutsPerPacket) const;

  ///
  /// \brief Initialize grid coordinates for bitmap generation.
  ///
  void initializeGridCoordinates();
};

} // namespace cbm

// GCOVR_EXCL_STOP
