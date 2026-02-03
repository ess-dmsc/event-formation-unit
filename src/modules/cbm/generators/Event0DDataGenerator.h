// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial EVENT_0D readout data for CBM beam monitor
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/time/ESSTime.h>
#include <modules/cbm/generators/CbmDataGenerator.h>
#include <cstdint>
#include <functional>

namespace cbm {

///
/// \class Event0DDataGenerator
/// \brief Generator for EVENT_0D beam monitor readouts.
///
/// Generates simple 0D event data with fixed ADC values and no position information.
///
class Event0DDataGenerator : public CbmDataGenerator {
public:
  /// \brief Constructor
  /// \param fiberId Fiber ID for readouts
  /// \param fenId FEN ID for readouts
  /// \param channelId Channel ID for readouts
  /// \param readoutTimeGenerator Function that returns (TimeHigh, TimeLow) tuple
  Event0DDataGenerator(
      uint8_t fiberId, uint8_t fenId, uint8_t channelId,
      std::function<std::pair<uint32_t, uint32_t>()> readoutTimeGenerator)
      : FiberId(fiberId), FenId(fenId), ChannelId(channelId),
        ReadoutTimeGenerator(readoutTimeGenerator) {}

  ///
  /// \brief Generate EVENT_0D readout data.
  ///
  /// \param dataPtr Pointer to the buffer where data should be written
  /// \param readoutsPerPacket Number of readouts to generate
  /// \param pulseTime Pulse time reference (optional, default is zero)
  ///
  void generateData(uint8_t *dataPtr, uint32_t readoutsPerPacket,
                    esstime::ESSTime pulseTime = esstime::ESSTime()) const override;

private:
  uint8_t FiberId;                                     ///< Fiber ID
  uint8_t FenId;                                       ///< FEN ID
  uint8_t ChannelId;                                   ///< Channel ID

  std::function<std::pair<uint32_t, uint32_t>()>
      ReadoutTimeGenerator;  ///< Function to generate readout times
};

} // namespace cbm

// GCOVR_EXCL_STOP
