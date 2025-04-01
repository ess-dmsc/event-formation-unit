// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
///                              WARNING
///
///                              ACHTUNG
///
///                              AVISO
///
///                              ADVARSEL
///
///                              DĖMESIO
///
///                              UWAGA
///
///
///
///          MODIFY THIS FILE ONLY AFTER UNDERSTANDING THE FOLLOWING
///
///
///   Any changes to non-static variable definitions will likely break h5 file
/// writing and compatibility. If you rename, reorder or change the type of any
/// of the member variables, you MUST:
///    A) Increment FormatVersion by 1
///    B) Ensure the hdf5 TypeTrait maps the struct correctly
///
/// If you cannot ensure the above, consult someone who can.
///
/// \file
///
/// \brief Readout struct for Caen event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdlib>
#include <string>

namespace Caen {

struct __attribute__((packed)) Readout {
  /// \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "loki_readouts"; }
  static uint16_t FormatVersion() { return 1; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint32_t PulseTimeHigh{0};
  uint32_t PulseTimeLow{0};
  uint32_t PrevPulseTimeHigh{0};
  uint32_t PrevPulseTimeLow{0};
  uint32_t EventTimeHigh{0};
  uint32_t EventTimeLow{0};
  uint16_t Unused{0}; // Pulse Height in debug mode
  int16_t AmpA{0};
  int16_t AmpB{0};
  int16_t AmpC{0};
  int16_t AmpD{0};
  uint8_t OutputQueue{0};
  uint8_t FiberId;
  uint8_t FENId;
  uint8_t Group{0};

  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  /// \brief prints values for to_string purposes
  std::string debug() const;
};

} // namespace Caen
