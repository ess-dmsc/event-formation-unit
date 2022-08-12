// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Small functions for bit- and byte- manipulations, graycode, reverse
/// bits
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

class BitMath {
public:
  inline static uint32_t gray2bin32(uint32_t num) {
    num = num ^ (num >> 16);
    num = num ^ (num >> 8);
    num = num ^ (num >> 4);
    num = num ^ (num >> 2);
    num = num ^ (num >> 1);
    return num;
  }

  /// \todo this is a hack to allow compilation of code from
  /// ROOT using cling (variant of clang) without c++14 support.
  inline static
#ifndef VMM_SDAT
      constexpr
#endif
      uint64_t
      NextPowerOfTwo(uint64_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;
    return n;
  }
};
