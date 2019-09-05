/** Copyright (C) 2019 European Spallation Source ERIC */

/** @file
 *
 *  \brief
 */

#include "PulseBuffer.h"
#include <algorithm>
#include <limits>

PulseBuffer::PulseBuffer(std::uint64_t TimeoutNS, size_t BufferSize)
    : Timeout(TimeoutNS), MaxPulses(BufferSize) {}

bool PulseBuffer::hasValidPulses() {
  if (Pulses.empty()) {
    return true;
  }
  while (true) {
    if (std::any_of(Pulses.begin(), Pulses.end(),
                    [](auto const &Item) { return Item.second.empty(); })) {
      return false;
    }
    std::sort(QueueList.begin(), QueueList.end(),
              [](auto const &A, auto const &B) {
                return A->front().ThresholdTimestampNS <
                       B->front().ThresholdTimestampNS;
              });
    auto EndEvent = (*(QueueList.end() - 1))->front();
    auto BeginEvent = (*QueueList.begin())->front();
    if ((EndEvent.ThresholdTimestampNS - BeginEvent.ThresholdTimestampNS) >
        Timeout) {
      (*QueueList.begin())->pop();
      ++DiscardedPulses;
    } else {
      break;
    }
  }
  return true;
}

void PulseBuffer::addChannel(ChannelID ID) {
  if (Pulses.find(ID) == Pulses.end()) {
    Pulses[ID] = PulseQueue();
    QueueList.push_back(&Pulses[ID]);
  }
}

void PulseBuffer::addPulse(const PulseParameters &Pulse) {
  if (Pulses.find(Pulse.Identifier) != Pulses.end()) {
    Pulses[Pulse.Identifier].push(Pulse);
    if (Pulses[Pulse.Identifier].size() > MaxPulses) {
      Pulses[Pulse.Identifier].pop();
      ++DiscardedPulses;
    }
  } else {
    ++InvalidPulses;
  }
}

std::vector<PulseParameters> PulseBuffer::getPulses() {
  std::vector<PulseParameters> ReturnVector;
  for (auto &PQ : Pulses) {
    ReturnVector.push_back(PQ.second.front());
    PQ.second.pop();
  }
  return ReturnVector;
}
