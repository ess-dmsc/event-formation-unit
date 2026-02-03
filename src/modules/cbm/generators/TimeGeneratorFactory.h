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
#include <generators/functiongenerators/DistributionGenerator.h>
#include <generators/functiongenerators/FixedValueGenerator.h>
#include <generators/functiongenerators/LinearGenerator.h>

#include <memory>

namespace cbm {

// Forward declaration
struct CbmGeneratorSettings;

///
/// \class TimeGeneratorFactory
/// \brief Factory class for creating time generator for CbmDataGenerator instances
///
/// This factory encapsulates the complex logic for creating the appropriate
/// CbmDataGenerator based on monitor type and settings, improving code
/// readability and maintainability.
///
class TimeGeneratorFactory {
public:
  ///
  /// \brief Creates the appropriate FunctionGenerator for readout time generation
  ///
  /// \param monitorType The type of CBM monitor
  /// \param frequency Pulse frequency in Hz
  /// \param numReadouts Number of readouts (used for IBM type)
  /// \return Unique pointer to the created FunctionGenerator
  /// \throws std::runtime_error if monitor type is unsupported
  ///
  static std::unique_ptr<FunctionGenerator> createTimeGenerator(
      CbmType monitorType, uint16_t frequency, uint32_t numReadouts = 0);
  };
 } // namespace cbm

// GCOVR_EXCL_STOP
 