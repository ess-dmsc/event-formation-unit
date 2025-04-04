// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
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
/// \file Hit2D.h
///
/// \brief  Hit2D struct for general clustering solution
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <string>
#include <limits>

struct __attribute__((packed)) Hit2D {
  // \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "efu_hits"; }
  static uint16_t FormatVersion() { return 0; }

  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint64_t time{0};
  uint16_t x_coordinate{0};
  uint16_t y_coordinate{0};
  uint16_t weight{0};
  /// !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  /// \brief prints values for debug purposes
  std::string to_string() const;

  static constexpr uint16_t InvalidCoord{std::numeric_limits<uint16_t>::max()};
  static constexpr uint8_t InvalidPlane{std::numeric_limits<uint8_t>::max()};
  static constexpr uint8_t PulsePlane{std::numeric_limits<uint8_t>::max() - 1};
};
