// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial readout data for BEER instrument
///        Event 2D data generation
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <cbm/generators/CbmDataGenerator.h>
#include <cbm/generators/Event2DDataGenerator.h>
#include <common/testutils/bitmaps/BitMaps.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <memory>
#include <string>

namespace beer {

///
/// \brief Struct representing the settings for the BeerGenerator.
///
// clang-format off
  struct BeerGeneratorSettings {
    uint8_t FiberId{0};                // Fiber ID (default 0 for BEER).
    uint8_t FenId{0};                  // The FEN ID.
    uint8_t ChannelId{0};              // The channel ID.
    uint16_t MaxXValue{512};           // Maximum X coordinate value for BEER detector used in 
                                       // random generator.
    uint16_t MaxYValue{512};           // Maximum Y coordinate value for BEER detector used in 
                                       // random generator.
    bool BeamMask{false};              // Flag for generating images.
    bool Randomise{false};             // Flag to Randomise the data.
  };
// clang-format on

///
/// \class ReadoutGenerator
/// \brief This class is responsible for generating Event 2D readout data for
/// BEER instrument.
///
/// The ReadoutGenerator class is a derived class of ReadoutGeneratorBase and
/// is used to generate Event 2D readout data for BEER instrument. It reuses
/// the CBM Event2DDataGenerator and data structures while generating data
/// for the BEER detector type.
///
class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  ///
  /// \brief Constructor for the ReadoutGenerator class.
  ///
  ReadoutGenerator();

  ///
  /// \brief Sets up a generator. Internal Buffers, socket, etc. are
  /// instantiated
  ///
  /// \param timeGenerator A unique pointer to a FunctionGenerator that
  /// provides time of flight distribution for readout time calculations. Use a
  /// DistributionGenerator implementation when neutron arrival follows a
  /// probability distribution, or a LinearDistribution when neutrons are
  /// expected at specific intervals.
  /// \throws std::runtime_error Header version is not V0 or V1.
  ///
  void initialize(std::unique_ptr<FunctionGenerator> &&timeGenerator) override;

private:
  BeerGeneratorSettings BeerSettings;

  /// Bitmap images used as neutron masks
  std::vector<const std::vector<std::pair<uint8_t, uint8_t>> *> mImages{};

  /// Data generator for Event 2D type
  std::unique_ptr<cbm::Event2DDataGenerator> DataGenerator{nullptr};

  ///
  /// \brief Generates the data for the ReadoutGenerator.
  ///
  void generateData() override;
};

} // namespace beer

// GCOVR_EXCL_STOP
