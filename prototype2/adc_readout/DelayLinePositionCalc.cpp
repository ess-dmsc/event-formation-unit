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
  return DelayLinePositionInterface::Origin;
}

DelayLinePosCalcInterface::DelayLinePosCalcInterface(std::uint64_t Timeout)
    : DelayLinePositionInterface(), EventTimeout(Timeout) {}

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
  for (auto &Pulse : PulseData) {
    Timestamps.emplace_back(Pulse.second.PeakTimestamp.GetTimeStampNS());
  }
  std::sort(Timestamps.begin(), Timestamps.end());
  return Timestamps.front() != 0 and
      Timestamps.back() - Timestamps.front() <= EventTimeout;
}

void DelayLinePosCalcInterface::reset() { PulseData.clear(); }

std::uint64_t DelayLinePosCalcInterface::getTimestamp() {
  auto Pulse =
      PulseData.find(DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  if (Pulse != PulseData.end()) {
    return Pulse->second.PeakTimestamp.GetTimeStampNS();
  }
  Pulse = PulseData.find(DelayLinePosCalcInterface::ChannelRole::FIRST);
  if (Pulse != PulseData.end()) {
    return Pulse->second.PeakTimestamp.GetTimeStampNS();
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
  return DelayLinePosCalcInterface::isValid() and
         PulseData.find(DelayLinePosCalcInterface::ChannelRole::FIRST) !=
             PulseData.end() and
         PulseData.find(DelayLinePosCalcInterface::ChannelRole::REFERENCE) !=
             PulseData.end();
}

int DelayLineAmpPosCalc::getPosition() {
  using Role = DelayLinePosCalcInterface::ChannelRole;
  if (PulseData.find(DelayLinePosCalcInterface::ChannelRole::SECOND) !=
      PulseData.end()) {
    return applyCalibration((PulseData[Role::FIRST].PeakAmplitude +
                             PulseData[Role::SECOND].PeakAmplitude) /
                            2.0);
  }
  return applyCalibration(PulseData[Role::FIRST].PeakAmplitude);
}

int DelayLineTimePosCalc::getPosition() {
  using Role = DelayLinePosCalcInterface::ChannelRole;
  if (PulseData.find(DelayLinePosCalcInterface::ChannelRole::SECOND) !=
      PulseData.end()) {
    auto TimeDiff1 = PulseData[Role::FIRST].PeakTimestamp.GetTimeStampNS() -
                     PulseData[Role::REFERENCE].PeakTimestamp.GetTimeStampNS();
    auto TimeDiff2 = PulseData[Role::SECOND].PeakTimestamp.GetTimeStampNS() -
                     PulseData[Role::REFERENCE].PeakTimestamp.GetTimeStampNS();
    return applyCalibration((TimeDiff1 + TimeDiff2) / 2.0);
  }
  return applyCalibration(
      PulseData[Role::FIRST].PeakTimestamp.GetTimeStampNS() -
      PulseData[Role::REFERENCE].PeakTimestamp.GetTimeStampNS());
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
