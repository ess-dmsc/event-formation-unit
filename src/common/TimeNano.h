/* Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time types for nano secs.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>

using TimePointNano = std::chrono::high_resolution_clock::time_point;
using TimeDurationNano = std::chrono::duration<size_t, std::nano>;