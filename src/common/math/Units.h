// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Common data-size unit helpers
//===----------------------------------------------------------------------===//

#pragma once

#include <cstddef>

namespace essmath::units {

inline constexpr std::size_t Byte = 1ULL;
inline constexpr std::size_t KiB = 1024ULL * Byte;
inline constexpr std::size_t MiB = 1024ULL * KiB;
inline constexpr std::size_t GiB = 1024ULL * MiB;
inline constexpr std::size_t TiB = 1024ULL * GiB;

} // namespace essmath::units

