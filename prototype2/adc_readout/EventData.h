// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Serialize neutron events by putting them into a flatbuffer
///        (header file).
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

struct EventData {
  std::uint64_t Timestamp{0};
  std::uint32_t EventId{0};
  std::uint32_t Amplitude{0};
  std::uint32_t PeakArea{0};
  std::uint32_t Background{0};
  std::uint64_t ThresholdTime{0};
  std::uint64_t PeakTime{0};

  bool operator==(EventData const &Other) const {
    return Timestamp == Other.Timestamp and EventId == Other.EventId and Amplitude == Other.Amplitude and
           PeakArea == Other.PeakArea and Background == Other.Background and ThresholdTime == Other.ThresholdTime and
           PeakTime == Other.PeakTime;
  }
};
