/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief
 */

#include "DelayLineEventFormation.h"
#include <common/Log.h>
#include <cstdlib>

using AxisType = AdcSettings::PositionSensingType;

std::unique_ptr<DelayLinePositionInterface> createCalculator(AxisType CalcType, int Timeout) {
  switch (CalcType) {
    case AxisType::AMPLITUDE:
      return
      std::make_unique<DelayLineAmpPosCalc>(Timeout);
      break;
    case AxisType::TIME:
      return
      std::make_unique<DelayLineAmpPosCalc>(Timeout);
      break;
    case AxisType::CONST: // Fall through
    default:
      break;
  }
  return std::make_unique<ConstDelayLinePosition>();
}

DelayLineEventFormation::DelayLineEventFormation(
    AdcSettings const &ReadoutSettings) {
  XAxisCalc = createCalculator(ReadoutSettings.XAxis, ReadoutSettings.EventTimeoutNS);
  XAxisCalc->setCalibrationValues(ReadoutSettings.XAxisCalibOffset,
                                  ReadoutSettings.XAxisCalibSlope);

  YAxisCalc = createCalculator(ReadoutSettings.YAxis, ReadoutSettings.EventTimeoutNS);
  YAxisCalc->setCalibrationValues(ReadoutSettings.YAxisCalibOffset,
                                  ReadoutSettings.YAxisCalibSlope);

  std::function<bool(DelayLinePosCalcInterface *)> IsAmpCalc =
      [](auto *Ptr) -> bool {
    if (nullptr == dynamic_cast<DelayLineAmpPosCalc *>(Ptr)) {
      LOG(INIT, Sev::Error, "Tried to assign pulses from a channel to an axis "
                            "for amplitude (TAC) processing but the axis is "
                            "assigned another type of processing.");
      return false;
    }
    return true;
  };

  std::function<bool(DelayLinePosCalcInterface *)> IsTimeCalc =
      [](auto *Ptr) -> bool {
    if (nullptr == dynamic_cast<DelayLineTimePosCalc *>(Ptr)) {
      LOG(INIT, Sev::Error, "Tried to assign pulses from a channel to an axis "
                            "for time processing but the axis is assigned "
                            "another type of processing.");
      return false;
    }
    return true;
  };

  std::function<bool(DelayLinePosCalcInterface *)> IsRefCalc =
      [](auto *Ptr) -> bool {
    if (nullptr == dynamic_cast<DelayLinePosCalcInterface *>(Ptr)) {
      LOG(INIT, Sev::Error, "Tried to assign pulses from a channel to an axis "
                            "for use as a time reference but the axis has "
                            "another type of processing.");
      return false;
    }
    return true;
  };
  using RoleParams = std::tuple<DelayLinePosCalcInterface *,
                              DelayLinePosCalcInterface::ChannelRole,
                              std::function<bool(DelayLinePosCalcInterface *)>>;
  std::multimap<AdcSettings::ChannelRole, RoleParams> ChannelRoleMap;
  
  auto addRole = [&ChannelRoleMap](auto Role, auto &AxisPtr, auto CAxisRole, auto CheckFunc) {
    ChannelRoleMap.emplace(Role, RoleParams{dynamic_cast<DelayLinePosCalcInterface *>(AxisPtr.get()), CAxisRole, CheckFunc});
  };
  using ChannelRole = AdcSettings::ChannelRole;
  using AxisRole = DelayLinePosCalcInterface::ChannelRole;
  
  // Add possible roles
  addRole(ChannelRole::AMPLITUDE_X_AXIS_1, XAxisCalc, AxisRole::FIRST, IsAmpCalc);
  addRole(ChannelRole::AMPLITUDE_X_AXIS_2, XAxisCalc, AxisRole::SECOND, IsAmpCalc);
  addRole(ChannelRole::AMPLITUDE_Y_AXIS_1, YAxisCalc, AxisRole::FIRST, IsAmpCalc);
  addRole(ChannelRole::AMPLITUDE_Y_AXIS_2, YAxisCalc, AxisRole::SECOND, IsAmpCalc);
  
  addRole(ChannelRole::TIME_X_AXIS_1, XAxisCalc, AxisRole::FIRST, IsTimeCalc);
  addRole(ChannelRole::TIME_X_AXIS_2, XAxisCalc, AxisRole::SECOND, IsTimeCalc);
  addRole(ChannelRole::TIME_Y_AXIS_1, YAxisCalc, AxisRole::FIRST, IsTimeCalc);
  addRole(ChannelRole::TIME_Y_AXIS_2, YAxisCalc, AxisRole::SECOND, IsTimeCalc);
  
  addRole(ChannelRole::REFERENCE_TIME, XAxisCalc, AxisRole::REFERENCE, IsRefCalc);
  addRole(ChannelRole::REFERENCE_TIME, YAxisCalc, AxisRole::REFERENCE, IsRefCalc);

  auto DoChannelRoleMapping = [&](ChannelID ID, auto Role) {
    auto PossibleRoles = ChannelRoleMap.equal_range(Role);
    for (auto CurrentRole = PossibleRoles.first;
         CurrentRole != PossibleRoles.second; ++CurrentRole) {
      auto RoleParams = CurrentRole->second;
      auto AxisPointer = std::get<0>(RoleParams);
      auto AxisRole = std::get<1>(RoleParams);
      auto AxisCheckFunction = std::get<2>(RoleParams);
      if (AxisCheckFunction(AxisPointer)) {
        AxisPointer->setChannelRole(ID, AxisRole);
        PulseHandlerMap.emplace(ID, AxisPointer);
      }
    }
  };
  
  // Apply roles
  DoChannelRoleMapping({0, 0}, ReadoutSettings.ADC1Channel1);
  DoChannelRoleMapping({0, 1}, ReadoutSettings.ADC1Channel2);
  DoChannelRoleMapping({0, 2}, ReadoutSettings.ADC1Channel3);
  DoChannelRoleMapping({0, 3}, ReadoutSettings.ADC1Channel4);
  
  DoChannelRoleMapping({1, 0}, ReadoutSettings.ADC2Channel1);
  DoChannelRoleMapping({1, 1}, ReadoutSettings.ADC2Channel2);
  DoChannelRoleMapping({1, 2}, ReadoutSettings.ADC2Channel3);
  DoChannelRoleMapping({1, 3}, ReadoutSettings.ADC2Channel4);
}

void DelayLineEventFormation::addPulse(const PulseParameters &Pulse) {
  auto PossiblePulseHandlers = PulseHandlerMap.equal_range(Pulse.Identifier);
  if (PossiblePulseHandlers.first == PulseHandlerMap.end()) {
    ++DiscardedDelayLinePulses;
    return;
  }
  ++ProcessedDelayLinePulses;
  for (auto CurrentHandler = PossiblePulseHandlers.first;
       CurrentHandler != PossiblePulseHandlers.second; ++CurrentHandler) {
    if (auto Calculator =
            dynamic_cast<DelayLinePosCalcInterface *>(CurrentHandler->second)) {
      Calculator->addPulse(Pulse);
    }
  }
}

bool DelayLineEventFormation::hasValidEvent() {
  return XAxisCalc->isValid() and YAxisCalc->isValid();
}

DelayLineEvent DelayLineEventFormation::popEvent() {
  if (not hasValidEvent()) {
    return {};
  }
  auto XEvent = XAxisCalc->popEvent();
  auto YEvent = YAxisCalc->popEvent();
  auto Amplitude = (XEvent.Amplitude + YEvent.Amplitude) / 2;
  auto XPos = XEvent.Position;
  auto YPos = YEvent.Position;
  auto Timestamp = XEvent.Timestamp;
  if (Timestamp == 0) {
    Timestamp = YEvent.Timestamp;
  }
  return {XPos, YPos, Amplitude, Timestamp};
}
