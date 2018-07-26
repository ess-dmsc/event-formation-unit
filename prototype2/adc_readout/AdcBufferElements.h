/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Circular buffer element.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

/// \brief Buffer used to store raw data from the UDP socket.
struct InData {
  static const int MaxLength = 10000;
  std::uint8_t Data[MaxLength] = {};
  unsigned int Length = 0;
};
