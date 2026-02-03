// Copyright (C) 2025-2026 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Factory for creating CbmDataGenerator instances - Implementation
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cbm/generators/DataGeneratorFactory.h>
#include <cbm/generators/ReadoutGenerator.h>
#include <generators/functiongenerators/DistributionGenerator.h>
#include <generators/functiongenerators/FixedValueGenerator.h>
#include <generators/functiongenerators/LinearGenerator.h>

namespace cbm {

std::unique_ptr<CbmDataGenerator> DataGeneratorFactory::createDataGenerator(
    const CbmGeneratorSettings &settings,
    uint16_t frequency,
    ReadoutFunction_t readoutTimeGenerator,
    const ImageVector_t &images) {

  if (settings.monitorType == CbmType::EVENT_0D) {
    return Event0DDataFactory(readoutTimeGenerator, settings);
  } else if (settings.monitorType == CbmType::EVENT_2D) {
    return Event2DDataFactory(readoutTimeGenerator, images, settings);
  } else if (settings.monitorType == CbmType::IBM) {
    return IBMDataFactory(readoutTimeGenerator, frequency, settings);
  } 

  throw std::runtime_error("Monitor type setting is invalid. Only "
    "EVENT_0D, EVENT_2D, IBM are allowed");
}

std::unique_ptr<Event0DDataGenerator> DataGeneratorFactory::Event0DDataFactory(
  ReadoutFunction_t readoutTimeGenerator,
  const CbmGeneratorSettings &settings) {

  return std::make_unique<Event0DDataGenerator>(
      settings.FiberId, settings.FenId, settings.ChannelId,
      readoutTimeGenerator);
}

std::unique_ptr<Event2DDataGenerator> DataGeneratorFactory::Event2DDataFactory(
    ReadoutFunction_t readoutTimeGenerator, const ImageVector_t &images, 
    const CbmGeneratorSettings &settings) {

    return std::make_unique<Event2DDataGenerator>(
        settings.FiberId, settings.FenId, settings.ChannelId,
        settings.MaxXValue, settings.MaxYValue, settings.BeamMask,
        settings.Randomise, readoutTimeGenerator, images);
}

std::unique_ptr<IBMDataGenerator> DataGeneratorFactory::IBMDataFactory(
    ReadoutFunction_t readoutTimeGenerator, uint16_t frequency, 
    const CbmGeneratorSettings &settings) {

    std::unique_ptr<FunctionGenerator> valueGenerator;
    if (settings.generatorType == GeneratorType::Distribution) {
      valueGenerator = DistributionFactory(frequency, settings);
    } else if (settings.generatorType == GeneratorType::Linear) {
      valueGenerator = LinearFactory(frequency, settings);
    } else if (settings.generatorType == GeneratorType::Fixed) {
      valueGenerator = FixValueFactory(settings);
    } else {
      throw std::runtime_error("Generator type setting is invalid. Only "
        "Distribution, Linear, Fixed are allowed");
    }
    return std::make_unique<IBMDataGenerator>(
        settings.FiberId, settings.FenId, settings.ChannelId,
        settings.NumReadouts, settings.NormFactor, settings.ShakeBeam,
        settings.Randomise, MAG_FACTOR, readoutTimeGenerator,
        std::move(valueGenerator));
}

std::unique_ptr<FixedValueGenerator> DataGeneratorFactory::FixValueFactory(
  const CbmGeneratorSettings &settings) {

      if (!settings.Value.has_value()) {
        throw std::runtime_error("Value must be provided for Fixed generator type");
      }
      return std::make_unique<FixedValueGenerator>(
          settings.Value.value() + settings.Offset);
}

std::unique_ptr<LinearGenerator> DataGeneratorFactory::LinearFactory(
  uint16_t frequency, const CbmGeneratorSettings &settings) {

      if (!settings.Gradient.has_value()) {
        throw std::runtime_error("Gradient must be provided for Linear generator type");
      }
      return std::make_unique<LinearGenerator>(
          frequency, settings.NumReadouts, settings.Gradient.value());
}

std::unique_ptr<DistributionGenerator> DataGeneratorFactory::DistributionFactory(
  uint16_t frequency, const CbmGeneratorSettings &settings) {

      auto GenMax = MILLISEC / frequency;
      if (settings.ShakeBeam) {
        GenMax += std::round(static_cast<float>(SHAKE_BEAM_US.second) / MILLISEC);
      }
      return std::make_unique<DistributionGenerator>(
          static_cast<double>(GenMax), settings.NumberOfBins);
  }

} // namespace cbm

// GCOVR_EXCL_STOP
