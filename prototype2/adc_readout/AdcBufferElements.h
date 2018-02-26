/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Circular buffer element.
 */

#pragma once

#include <cstdint>

struct InData {
  static const int MaxLength = 2048;
  std::uint8_t Data[MaxLength] = {};
  unsigned int Length = 0;
};
