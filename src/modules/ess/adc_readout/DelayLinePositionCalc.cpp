/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief
 */

#include "DelayLinePositionCalc.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <vector>

int ConstDelayLinePosition::getPosition() {
  return std::lround(DelayLinePositionInterface::Calib.Origin);
}

DelayLinePosCalcInterface::DelayLinePosCalcInterface(std::uint64_t Timeout)
    : EventTimeout(Timeout) {}

void DelayLinePosCalcInterface::setChannelRole(ChannelID const &ID,
                                               ChannelRole const &Role) {
  RoleMap[ID] = Role;
}

void DelayLinePosCalcInterface::addPulse(const PulseParameters &Pulse) {
  auto CurrentRole = RoleMap.find(Pulse.Identifier);
  if (CurrentRole == RoleMap.end()) {
    return;
  }
  PulseData[CurrentRole->second] = Pulse;
}

bool DelayLinePosCalcInterface::isValid() {
  if (PulseData.size() != RoleMap.size() or RoleMap.empty()) {
    return false;
  }
  std::vector<std::uint64_t> Timestamps;
  std::transform(PulseData.begin(), PulseData.end(),
                 std::back_inserter(Timestamps),
                 [](auto &Pulse) { return Pulse.second.ThresholdTimestampNS; });
  std::sort(Timestamps.begin(), Timestamps.end());
  return Timestamps.front() != 0 and
         Timestamps.back() - Timestamps.front() <= EventTimeout;
}

void DelayLinePosCalcInterface::reset() { PulseData.clear(); }

std::uint64_t DelayLinePosCalcInterface::getTimestamp() {
  auto Pulse =
      PulseData.find(DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  if (Pulse != PulseData.end()) {
    return Pulse->second.ThresholdTimestampNS;
  }
  Pulse = PulseData.find(DelayLinePosCalcInterface::ChannelRole::FIRST);
  if (Pulse != PulseData.end()) {
    return Pulse->second.ThresholdTimestampNS;
  }
  return 0;
}

DelayLineAmpPosCalc::DelayLineAmpPosCalc(std::uint64_t Timeout)
    : DelayLinePosCalcInterface(Timeout) {}

bool DelayLineAmpPosCalc::isValid() {
  return DelayLinePosCalcInterface::isValid() and
         PulseData.find(DelayLinePosCalcInterface::ChannelRole::FIRST) !=
             PulseData.end();
}

DelayLineTimePosCalc::DelayLineTimePosCalc(std::uint64_t Timeout)
    : DelayLinePosCalcInterface(Timeout) {}

bool DelayLineTimePosCalc::isValid() {
  if (PulseData.size() != RoleMap.size() or RoleMap.empty()) {
    return false;
  }
  std::vector<PulseParameters> SortedPulses;
  std::transform(PulseData.begin(), PulseData.end(),
                 std::back_inserter(SortedPulses),
                 [](auto &Pulse) { return Pulse.second; });
  std::sort(SortedPulses.begin(), SortedPulses.end(),
            [](auto &Pulse1, auto &Pulse2) {
              return Pulse1.ThresholdTimestampNS < Pulse2.ThresholdTimestampNS;
            });

  // Note: A lot of short circuit evaluation below. Take care when/if changing
  // order of logical tests.
  return PulseData.find(ChannelRole::REFERENCE) != PulseData.end() and
         PulseData.find(ChannelRole::FIRST) != PulseData.end() and
         SortedPulses.front().ThresholdTimestampNS != 0 and
         SortedPulses.back().ThresholdTimestampNS -
                 SortedPulses.front().ThresholdTimestampNS <=
             EventTimeout and
         (RoleMap[SortedPulses.front().Identifier] == ChannelRole::REFERENCE or
          SortedPulses.front().ThresholdTimestampNS ==
              SortedPulses.at(1).ThresholdTimestampNS);
}

int DelayLineAmpPosCalc::getPosition() {
  using Role = DelayLinePosCalcInterface::ChannelRole;
  if (PulseData.find(DelayLinePosCalcInterface::ChannelRole::SECOND) !=
      PulseData.end()) {
    return applyCalibration(PulseData[Role::FIRST].PeakAmplitude -
                            PulseData[Role::SECOND].PeakAmplitude);
  }
  return applyCalibration(PulseData[Role::FIRST].PeakAmplitude);
}

int DelayLineTimePosCalc::getPosition() {
  using Role = DelayLinePosCalcInterface::ChannelRole;
  auto TimeDiff1 = static_cast<std::int32_t>(
      PulseData[Role::FIRST].ThresholdTimestampNS -
      PulseData[Role::REFERENCE].ThresholdTimestampNS);
  if (PulseData.find(DelayLinePosCalcInterface::ChannelRole::SECOND) !=
      PulseData.end()) {
    auto TimeDiff2 = static_cast<std::int32_t>(
        PulseData[Role::SECOND].ThresholdTimestampNS -
        PulseData[Role::REFERENCE].ThresholdTimestampNS);
    return applyCalibration(TimeDiff1 - TimeDiff2);
  }
  return applyCalibration(TimeDiff1);
}

int DelayLineTimePosCalc::getAmplitude() {
  using Role = DelayLinePosCalcInterface::ChannelRole;
  return PulseData[Role::REFERENCE].PeakArea;
}

int DelayLineAmpPosCalc::getAmplitude() {
  using Role = DelayLinePosCalcInterface::ChannelRole;
  if (PulseData.find(DelayLinePosCalcInterface::ChannelRole::REFERENCE) !=
      PulseData.end()) {
    return PulseData[Role::REFERENCE].PeakArea;
  }
  return 0;
}
