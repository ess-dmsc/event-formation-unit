/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Channel ID struct.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <functional>

struct ChannelID {
  std::uint16_t SourceID;
  std::uint16_t ChannelNr;
  bool operator==(ChannelID const &Other) const {
    return Other.ChannelNr == ChannelNr and Other.SourceID == SourceID;
  };
  bool operator<(ChannelID const &Other) const {
    const auto MaxNrOfChannels = 4;
    return ChannelNr + SourceID * MaxNrOfChannels <
           Other.ChannelNr + Other.SourceID * MaxNrOfChannels;
  };
};

namespace std {
template <> struct hash<ChannelID> {
  std::size_t operator()(const ChannelID &ID) const {
    return std::hash<std::uint32_t>{}(
        static_cast<uint32_t>(ID.ChannelNr) |
        (static_cast<uint32_t>(ID.SourceID) << 16));
  }
};
}
