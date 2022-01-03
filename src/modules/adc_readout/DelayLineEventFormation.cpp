/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief
 */

#include "DelayLineEventFormation.h"
#include <common/debug/Log.h>
#include <cstdlib>

using AxisType = AdcSettings::PositionSensingType;

std::unique_ptr<DelayLinePositionInterface> createCalculator(AxisType CalcType,
                                                             int Timeout) {
  switch (CalcType) {
  case AxisType::AMPLITUDE:
    return std::make_unique<DelayLineAmpPosCalc>(Timeout);
    break;
  case AxisType::TIME:
    return std::make_unique<DelayLineTimePosCalc>(Timeout);
    break;
  case AxisType::CONST: // Fall through
  default:
    break;
  }
  return std::make_unique<ConstDelayLinePosition>();
}

template <typename Type, typename PtrType>
bool checkType(PtrType Ptr, std::string const &ErrorString) {
  if (nullptr == dynamic_cast<Type *>(Ptr)) {
    LOG(INIT, Sev::Error, ErrorString);
    return false;
  }
  return true;
}

void DelayLineEventFormation::DoChannelRoleMapping(
    ChannelID ID, AdcSettings::ChannelRole Role) {
  std::string AmpError = "Tried to assign pulses from a channel to an axis "
                         "for amplitude (TAC) processing but the axis is "
                         "assigned another type of processing.";
  auto IsAmpCheck = [AmpError](auto *Ptr) {
    return checkType<DelayLineAmpPosCalc>(Ptr, AmpError);
  };
  std::string TimeError = "Tried to assign pulses from a channel to an axis "
                          "for amplitude (TAC) processing but the axis is "
                          "assigned another type of processing.";
  auto IsTimeCheck = [TimeError](auto *Ptr) {
    return checkType<DelayLineTimePosCalc>(Ptr, TimeError);
  };
  std::string RefError = "Tried to assign pulses from a channel to an axis "
                         "for use as a time reference but the axis has "
                         "another type of processing.";
  auto IsRefCheck = [RefError](auto *Ptr) {
    return checkType<DelayLinePosCalcInterface>(Ptr, RefError);
  };

  using RoleParams =
      std::tuple<DelayLinePosCalcInterface *,
                 DelayLinePosCalcInterface::ChannelRole,
                 std::function<bool(DelayLinePosCalcInterface *)>>;
  std::multimap<AdcSettings::ChannelRole, RoleParams> ChannelRoleMap;

  auto addRole = [&ChannelRoleMap](auto Role, auto &AxisPtr, auto CAxisRole,
                                   auto CheckFunc) {
    ChannelRoleMap.emplace(
        Role,
        RoleParams{dynamic_cast<DelayLinePosCalcInterface *>(AxisPtr.get()),
                   CAxisRole, CheckFunc});
  };
  using ChannelRole = AdcSettings::ChannelRole;
  using AxisRole = DelayLinePosCalcInterface::ChannelRole;

  // Add possible roles
  addRole(ChannelRole::AMPLITUDE_X_AXIS_1, XAxisCalc, AxisRole::FIRST,
          IsAmpCheck);
  addRole(ChannelRole::AMPLITUDE_X_AXIS_2, XAxisCalc, AxisRole::SECOND,
          IsAmpCheck);
  addRole(ChannelRole::AMPLITUDE_Y_AXIS_1, YAxisCalc, AxisRole::FIRST,
          IsAmpCheck);
  addRole(ChannelRole::AMPLITUDE_Y_AXIS_2, YAxisCalc, AxisRole::SECOND,
          IsAmpCheck);

  addRole(ChannelRole::TIME_X_AXIS_1, XAxisCalc, AxisRole::FIRST, IsTimeCheck);
  addRole(ChannelRole::TIME_X_AXIS_2, XAxisCalc, AxisRole::SECOND, IsTimeCheck);
  addRole(ChannelRole::TIME_Y_AXIS_1, YAxisCalc, AxisRole::FIRST, IsTimeCheck);
  addRole(ChannelRole::TIME_Y_AXIS_2, YAxisCalc, AxisRole::SECOND, IsTimeCheck);

  addRole(ChannelRole::REFERENCE_TIME, XAxisCalc, AxisRole::REFERENCE,
          IsRefCheck);
  addRole(ChannelRole::REFERENCE_TIME, YAxisCalc, AxisRole::REFERENCE,
          IsRefCheck);

  auto PossibleRoles = ChannelRoleMap.equal_range(Role);
  for (auto CurrentRole = PossibleRoles.first;
       CurrentRole != PossibleRoles.second; ++CurrentRole) {
    auto RoleParams = CurrentRole->second;
    auto AxisPointer = std::get<0>(RoleParams);
    auto AxisRole = std::get<1>(RoleParams);
    auto AxisCheckFunction = std::get<2>(RoleParams);
    if (AxisCheckFunction(AxisPointer)) {
      Buffer.addChannel(ID);
      AxisPointer->setChannelRole(ID, AxisRole);
      PulseHandlerMap.emplace(ID, AxisPointer);
    }
  }
}

DelayLineEventFormation::DelayLineEventFormation(
    AdcSettings const &ReadoutSettings)
    : XAxisCalc(createCalculator(ReadoutSettings.XAxis,
                                 ReadoutSettings.EventTimeoutNS)),
      YAxisCalc(createCalculator(ReadoutSettings.YAxis,
                                 ReadoutSettings.EventTimeoutNS)),
      Buffer(ReadoutSettings.EventTimeoutNS) {
  XAxisCalc->setCalibrationValues(ReadoutSettings.XAxisCalibOffset,
                                  ReadoutSettings.XAxisCalibSlope);

  YAxisCalc->setCalibrationValues(ReadoutSettings.YAxisCalibOffset,
                                  ReadoutSettings.YAxisCalibSlope);

  // Apply roles
  DoChannelRoleMapping({0, 0}, ReadoutSettings.ADC1Channel1);
  DoChannelRoleMapping({0, 1}, ReadoutSettings.ADC1Channel2);
  DoChannelRoleMapping({0, 2}, ReadoutSettings.ADC1Channel3);
  DoChannelRoleMapping({0, 3}, ReadoutSettings.ADC1Channel4);
}

void DelayLineEventFormation::addPulse(const PulseParameters &NewPulse) {
  auto PossiblePulseHandlers = PulseHandlerMap.equal_range(NewPulse.Identifier);
  if (PossiblePulseHandlers.first == PulseHandlerMap.end()) {
    ++DiscardedDelayLinePulses;
    return;
  }
  ++ProcessedDelayLinePulses;
  Buffer.addPulse(NewPulse);
  if (Buffer.hasValidPulses()) {
    auto CPulses = Buffer.getPulses();
    for (auto &CPulse : CPulses) {
      PossiblePulseHandlers = PulseHandlerMap.equal_range(CPulse.Identifier);
      for (auto CurrentHandler = PossiblePulseHandlers.first;
           CurrentHandler != PossiblePulseHandlers.second; ++CurrentHandler) {
        if (auto Calculator = dynamic_cast<DelayLinePosCalcInterface *>(
                CurrentHandler->second)) {
          Calculator->addPulse(CPulse);
        }
      }
    }
    if (not hasValidEvent()) {
      std::cout << "Problem!" << std::endl;
    }
  }
}

bool DelayLineEventFormation::hasValidEvent() {
  auto a = XAxisCalc->isValid();
  auto b = YAxisCalc->isValid();
  return a and b;
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
