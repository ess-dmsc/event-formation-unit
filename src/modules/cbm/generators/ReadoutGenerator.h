// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
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
#include <cbm/generators/GeneratorType.h>
#include <cbm/geometry/Parser.h>
#include <common/testutils/DataFuzzer.h>
#include <common/time/ESSTime.h>
#include <cstdint>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <generators/functiongenerators/FunctionGenerator.h>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace cbm {

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
  ///
  /// \brief Struct representing the settings for the CbmGenerator.
  ///

  // clang-format off
  struct CbmGeneratorSettings {
    CbmType monitorType{CbmType::EVENT_0D}; //< The type of monitor.
    uint8_t FenId{0};                  //< The FEN ID.
    uint8_t ChannelId{0};              //< The channel ID.
    uint32_t Offset{0};                //< The offset value.
    bool ShakeBeam{false};             //< Flag to shake the beam.
    bool Randomise{false};             //< Flag to Randomise the data.
    GeneratorType generatorType{
       GeneratorType::Distribution};   // The generator type.
    std::optional<uint32_t> Value;     // The optional value.
    std::optional<double> Gradient;  // The optional gradient.
    uint32_t NumberOfBins{512};        // The number of bins.
    uint32_t NumReadouts{5952};         // The number of readouts per pulse. 12us = 5952 for 14hz
  } cbmSettings;
  // clang-format on

  ///
  /// \brief Constructor for the ReadoutGenerator class.
  ///
  ReadoutGenerator();

private:
  // Beam monitors are always on logical fiber 22 (ring 11) and fen 0
  static constexpr uint8_t CBM_FIBER_ID = 22;
  static constexpr int MILLISEC = 1e3;

  // Shake beam time in microseconds range
  static constexpr std::pair<int, int> SHAKE_BEAM_US = {500, 1500};

  esstime::TimeDurationNano RandomTimeDriftNS{
      0}; // Variable to store the random time drift for shaking the beam.

  std::unique_ptr<FunctionGenerator> Generator{nullptr}; // The function
                                                         //  generator.

  // Initialize random generator with a seed from std::random_device
  std::minstd_rand RandomGenerator{std::random_device{}()};
  // Initialize the distribution with SHAKE_BEAM_US range
  std::uniform_int_distribution<int> BeamShakeDistMs{SHAKE_BEAM_US.first,
                                                     SHAKE_BEAM_US.second};
  std::uniform_int_distribution<int> NoiseDist{0, 50};

  ///
  /// \brief Generates the data for the ReadoutGenerator.
  ///
  void generateData() override;

  ///
  /// \brief Generates the IBM data for the ReadoutGenerator.
  ///
  /// \param dataPtr Pointer to the data buffer.
  ///
  void generateIBMData(uint8_t *dataPtr);

  ///
  /// \brief Generates the 0D event data for the ReadoutGenerator.
  ///
  /// \param dataPtr Pointer to the data buffer.
  ///
  void generateEvent0DData(uint8_t *dataPtr);

  ///
  /// \brief Generates the distribution value for the ReadoutGenerator.
  ///
  /// \param cbmReadout Pointer to the CbmReadout object.
  ///
  void distributionValueGenerator(Parser::CbmReadout *cbmReadout);

  ///
  /// \brief Generates the linear value for the ReadoutGenerator.
  ///
  /// \param cbmReadout Pointer to the CbmReadout object.
  ///
  void linearValueGenerator(Parser::CbmReadout *cbmReadout);

  ///
  /// \brief Generates the fixed value for the ReadoutGenerator.
  ///
  /// \param cbmReadout Pointer to the CbmReadout object.
  ///
  void fixedValueGenerator(Parser::CbmReadout *cbmReadout);
};

} // namespace cbm

// GCOVR_EXCL_STOP
