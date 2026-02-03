// Copyright (C) 2025-2026 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Factory for creating CbmDataGenerator instances
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <cbm/CbmTypes.h>
#include <cbm/generators/CbmDataGenerator.h>
#include <cbm/generators/Event0DDataGenerator.h>
#include <cbm/generators/Event2DDataGenerator.h>
#include <cbm/generators/GeneratorType.h>
#include <cbm/generators/IBMDataGenerator.h>
#include <generators/functiongenerators/DistributionGenerator.h>
#include <generators/functiongenerators/FixedValueGenerator.h>
#include <generators/functiongenerators/LinearGenerator.h>

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cbm {

// Forward declaration
struct CbmGeneratorSettings;

///
/// \class DataGeneratorFactory
/// \brief Factory class for creating CbmDataGenerator instances
///
/// This factory encapsulates the complex logic for creating the appropriate
/// CbmDataGenerator based on monitor type and settings, improving code
/// readability and maintainability.
///
class DataGeneratorFactory {
public:
  using ReadoutFunction_t = std::function<std::pair<uint32_t, uint32_t>()>;
  using Coordinates_t = std::pair<uint8_t, uint8_t>;
  using Image_t = std::vector<Coordinates_t>;
  using ImageVector_t = std::vector<const Image_t*>;

  ///
  /// \brief Creates the appropriate CbmDataGenerator based on settings
  ///
  /// \param settings CBM generator settings containing monitor type and configuration
  /// \param frequency Pulse frequency in Hz
  /// \param readoutTimeGenerator Function that generates readout time tuples (TimeHigh, TimeLow)
  /// \param images Vector of bitmap images for Event2D mask mode (optional)
  /// \return Unique pointer to the created CbmDataGenerator
  /// \throws std::runtime_error if monitor type is unsupported or required parameters are missing
  ///
  static std::unique_ptr<CbmDataGenerator> createDataGenerator(
      const CbmGeneratorSettings &settings,
      uint16_t frequency,
      ReadoutFunction_t readoutTimeGenerator,
      const ImageVector_t &images = {});

private:

  /// \brief 0D event data factory
  /// \param readoutTimeGenerator Function that generates readout time tuples (TimeHigh, TimeLow)
  /// \param settings CBM generator settings containing monitor type and configuration
  /// \return unique pointer to a Event0DDataGenerator object
  static std::unique_ptr<Event0DDataGenerator> Event0DDataFactory(
    ReadoutFunction_t readoutTimeGenerator,
    const CbmGeneratorSettings &settings);

  /// \brief 2D event data factory
  /// \param readoutTimeGenerator Function that generates readout time tuples (TimeHigh, TimeLow)
  /// \param images Vector of bitmap images for Event2D mask mode (optional)
  /// \param settings CBM generator settings containing monitor type and configuration
  /// \return unique pointer to a Event2DDataGenerator object
  static std::unique_ptr<Event2DDataGenerator> Event2DDataFactory(
    ReadoutFunction_t readoutTimeGenerator, const ImageVector_t &images, 
    const CbmGeneratorSettings &settings);

  /// \brief IBM event data factory
  /// \param readoutTimeGenerator Function that generates readout time tuples (TimeHigh, TimeLow)
  /// \param frequency Pulse frequency in Hz
  /// \param settings CBM generator settings containing monitor type and configuration
  /// \return unique pointer to a IBMDataGenerator object
  static std::unique_ptr<IBMDataGenerator> IBMDataFactory(
    ReadoutFunction_t readoutTimeGenerator, uint16_t frequency, 
    const CbmGeneratorSettings &settings);

  /// \brief Fixed value data generator
  /// \param settings CBM generator settings containing monitor type and configuration
  /// \return FixedValueGenerator object
  static std::unique_ptr<FixedValueGenerator> FixValueFactory(
    const CbmGeneratorSettings &settings);

  /// \brief Linear data distribution data generator
  /// \param frequency Pulse frequency in Hz
  /// \param settings CBM generator settings containing monitor type and configuration
  /// \return FixedValueGenerator object
  static std::unique_ptr<LinearGenerator> LinearFactory(
    uint16_t frequency, const CbmGeneratorSettings &settings);

  /// \brief Data selected from a fixed distribution 
  /// \param frequency Pulse frequency in Hz
  /// \param settings CBM generator settings containing monitor type and configuration
  /// \return FixedValueGenerator object
  static std::unique_ptr<DistributionGenerator> DistributionFactory(
    uint16_t frequency, const CbmGeneratorSettings &settings);

  static constexpr int MILLISEC = 1e3;
  static constexpr std::pair<int, int> SHAKE_BEAM_US = {500, 1500};
  static constexpr double MAG_FACTOR = 1000.0;

};

} // namespace cbm

// GCOVR_EXCL_STOP
