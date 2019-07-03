/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
///
//===----------------------------------------------------------------------===//

#pragma once

#include "ChannelID.h"
#include "PulseParameters.h"
#include <cstdint>
#include <queue>
#include <map>

class PulseBuffer {
public:
  PulseBuffer(std::uint64_t TimeoutNS, size_t BufferSize = 1000);
  void addChannel(ChannelID ID);
  void addPulse(PulseParameters const &Pulse);
  bool hasValidPulses();
  std::vector<PulseParameters> getPulses();
  auto getDiscardedPulses() const {return DiscardedPulses;}
  auto getInvalidPulses() const {return InvalidPulses;}
private:
  std::int64_t DiscardedPulses{0};
  std::int64_t InvalidPulses{0};
  std::uint64_t Timeout;
  size_t MaxPulses;
  using PulseQueue = std::queue<PulseParameters>;
  std::map<ChannelID, PulseQueue> Pulses;
  std::vector<PulseQueue*> QueueList;
};
