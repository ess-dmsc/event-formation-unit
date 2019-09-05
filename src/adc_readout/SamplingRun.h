/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple struct for containing pulse data.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcTimeStamp.h"
#include <cstdint>
#include <vector>

/// \brief Data stored in this struct represents a (properly parsed) sampling
/// run.
struct SamplingRun {
  SamplingRun() = default;
  explicit SamplingRun(size_t ReserveElements) noexcept
      : Data(ReserveElements) {
    Data.clear();
  }
  ~SamplingRun() = default;
  SamplingRun(SamplingRun const &) = default;
  explicit SamplingRun(const SamplingRun &&Other)
      : TimeStamp(Other.TimeStamp), Identifier(Other.Identifier),
        OversamplingFactor(Other.OversamplingFactor),
        Data(std::move(Other.Data)) {}
  SamplingRun &operator=(const SamplingRun &) = default;
  RawTimeStamp TimeStamp;
  RawTimeStamp ReferenceTimestamp;
  void reset() {
    Data.clear();
    OversamplingFactor = 1;
    TimeStamp.Seconds = 0;
    TimeStamp.SecondsFrac = 0;
    Identifier.ChannelNr = 0;
    Identifier.SourceID = 0;
  }
  ChannelID Identifier{0, 0};
  std::uint16_t OversamplingFactor{1};
  std::vector<std::uint16_t> Data;
};
