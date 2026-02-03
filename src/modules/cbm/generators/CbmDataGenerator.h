// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Abstract base class for CBM data generators
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/time/ESSTime.h>
#include <cstdint>

namespace cbm {

///
/// \class CbmDataGenerator
/// \brief Abstract base class for generating different types of CBM readout data.
///
/// This class defines the interface for generating specific CBM data types.
/// Different detector types (EVENT_0D, EVENT_2D, IBM) can implement their
/// own data generation strategies by inheriting from this class.
///
class CbmDataGenerator {
public:
  /// \brief Virtual destructor
  virtual ~CbmDataGenerator() = default;

  ///
  /// \brief Generate CBM readout data.
  ///
  /// \param dataPtr Pointer to the buffer where data should be written
  /// \param readoutsPerPacket Number of readouts to generate in this packet
  /// \param pulseTime Pulse time reference (optional, default is zero)
  ///
  virtual void generateData(uint8_t *dataPtr, uint32_t readoutsPerPacket,
                            esstime::ESSTime pulseTime = esstime::ESSTime()) const = 0;
};

} // namespace cbm

// GCOVR_EXCL_STOP
