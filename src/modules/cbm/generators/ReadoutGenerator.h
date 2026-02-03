// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial readout data for CBM beam monitor types
///        based on TTLMon ICD
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <cbm/CbmTypes.h>
#include <cbm/generators/CbmDataGenerator.h>
#include <cbm/generators/Event0DDataGenerator.h>
#include <cbm/generators/Event2DDataGenerator.h>
#include <cbm/generators/GeneratorType.h>
#include <cbm/generators/IBMDataGenerator.h>
#include <cbm/readout/Parser.h>
#include <common/testutils/bitmaps/BitMaps.h>
#include <common/time/ESSTime.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <generators/functiongenerators/FunctionGenerator.h>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace cbm {

///
/// \brief Struct representing the settings for the CbmGenerator.
///
// clang-format off
struct CbmGeneratorSettings {
  CbmType monitorType{CbmType::EVENT_0D}; // The type of monitor.
  uint8_t FiberId{22};               // Fiber ID. Beam monitors are always on logical  
                                     // fiber 22 (ring 11) and fen 0.
  uint8_t FenId{0};                  // The FEN ID.
  uint8_t ChannelId{0};              // The channel ID.
  uint32_t Offset{0};                // The offset value.
  bool BeamMask{false};              // Flag for generating images on 2D CBM.
  bool ShakeBeam{false};             // Flag to shake the beam.
  bool Randomise{false};             // Flag to Randomise the data.
  GeneratorType generatorType{
     GeneratorType::Distribution};   // The generator type.
  std::optional<uint32_t> Value;     // The optional value.
  std::optional<double> Gradient;    // The optional gradient.
  uint32_t NumberOfBins{512};        // The number of bins.
  uint32_t NumReadouts{5952};        // The number of readouts per pulse. 12us = 5952 for 14hz.
  uint16_t MaxXValue{512};           // Maximum X coordinate value for 2D Beam monitor used in 
                                     // random generator.
  uint16_t MaxYValue{512};           // Maximum Y coordinate value for 2D Beam monitor used in 
                                     // random generator.
  uint8_t NormFactor{1};             // Factor used for beam monitor histogram 
                                     // normalization. Default = 1
};
// clang-format on

///
/// \class ReadoutGenerator
/// \brief This class is responsible for generating readout data for beam
/// monitors.
///
/// The ReadoutGenerator class is a derived class of ReadoutGeneratorBase and
/// is used to generate readout data for beam monitors. It provides different
/// methods for generating data based on the monitor type and generator type.
/// The generated data can be used for various purposes such as testing and
/// analysis.
///
/// \note This class assumes that the beam monitors are always on logical
/// fiber 22 (ring 11) and fen 0.
///
class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  CbmGeneratorSettings cbmSettings;

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
  static constexpr int MILLISEC = 1e3;
  static constexpr std::pair<int, int> SHAKE_BEAM_US = {500, 1500};

  /// Bitmap images used as neutron masks
  std::vector<const std::vector<std::pair<uint8_t, uint8_t>> *> mImages{};

  /// Data generators for different CBM types
  std::unique_ptr<CbmDataGenerator> DataGenerator{nullptr};

  ///
  /// \brief Generates the data for the ReadoutGenerator.
  ///
  void generateData() override;
};

} // namespace cbm

// GCOVR_EXCL_STOP
