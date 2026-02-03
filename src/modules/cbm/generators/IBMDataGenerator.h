// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial IBM readout data for CBM beam monitor
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/time/ESSTime.h>
#include <modules/cbm/generators/CbmDataGenerator.h>
#include <cstdint>
#include <functional>
#include <generators/functiongenerators/FunctionGenerator.h>
#include <memory>
#include <random>

namespace cbm {

///
/// \class IBMDataGenerator
///
/// \class IBMDataGenerator
/// \brief Generator for IBM beam monitor readouts.
///
/// Generates IBM data with various value generation strategies:
/// - Distribution: Uses a distribution function
/// - Linear: Uses a linear function with gradient
/// - Fixed: Uses a fixed value
///
class IBMDataGenerator : public CbmDataGenerator {
public:
  /// \brief Constructor
  /// \param fiberId Fiber ID for readouts
  /// \param fenId FEN ID for readouts
  /// \param channelId Channel ID for readouts
  /// \param numReadouts Number of readouts per pulse
  /// \param normFactor Normalization factor for ADC values
  /// \param shakeBeam Enable random drift for beam position
  /// \param randomise Add noise to values
  /// \param magFactor Multiplication factor for generated values
  /// \param readoutTimeGenerator Function that returns (TimeHigh, TimeLow)
  /// nanoseconds 
  /// \param valueGenerator Pre-constructed function generator
  /// (ownership transferred)
  IBMDataGenerator(
      uint8_t fiberId, uint8_t fenId, uint8_t channelId, uint32_t numReadouts,
      uint8_t normFactor, bool shakeBeam, bool randomise, double magFactor,
      std::function<std::pair<uint32_t, uint32_t>()> readoutTimeGenerator,
      std::unique_ptr<FunctionGenerator> valueGenerator);

  ///
  /// \brief Generate IBM readout data.
  ///
  /// \param dataPtr Pointer to the buffer where data should be written
  /// \param readoutsPerPacket Number of readouts to generate
  /// \param pulseTime Pulse time reference (optional, default is zero)
  ///
  void generateData(uint8_t *dataPtr, uint32_t readoutsPerPacket,
                    esstime::ESSTime pulseTime = esstime::ESSTime()) const override;

private:
  uint8_t FiberId;      ///< Fiber ID
  uint8_t FenId;        ///< FEN ID
  uint8_t ChannelId;    ///< Channel ID
  uint32_t NumReadouts; ///< Number of readouts per pulse
  uint8_t NormFactor;   ///< Normalization factor
  bool ShakeBeam;       ///< Enable beam shaking
  bool Randomise;       ///< Add noise
  double MagFactor;     ///< Multiplication factor for the value generator

  std::function<std::pair<uint32_t, uint32_t>()>
      ReadoutTimeGenerator; ///< Function to generate readout times

  // Generator state
  std::unique_ptr<FunctionGenerator> ValueGenerator;
  mutable uint32_t NumberOfReadoutsGenerated{0};

  // Random number generation
  static constexpr int MILLISEC = 1e3;
  static constexpr std::pair<int, int> SHAKE_BEAM_US = {1500, 5000};

  mutable esstime::ESSTime LastPulseTime{0, 0};
  mutable double PulseDriftMs{0.0};
  mutable std::minstd_rand RandomGenerator{std::random_device{}()};
  mutable std::uniform_int_distribution<int> BeamShakeDistUs{SHAKE_BEAM_US.first,
                                                     SHAKE_BEAM_US.second};
  mutable std::uniform_int_distribution<int> NoiseDist{0, 50};
};

} // namespace cbm

// GCOVR_EXCL_STOP
