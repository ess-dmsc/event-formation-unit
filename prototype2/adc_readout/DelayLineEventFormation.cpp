/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief
 */

#include "DelayLineEventFormation.h"
#include <common/Log.h>
#include <cstdlib>

DelayLineEventFormation::DelayLineEventFormation(
    AdcSettings const &ReadoutSettings) {
  switch (ReadoutSettings.XAxis) {
  case AdcSettings::PositionSensingType::AMPLITUDE:
    XAxisCalc =
        std::make_unique<DelayLineAmpPosCalc>(ReadoutSettings.EventTimeoutNS);
    break;
  case AdcSettings::PositionSensingType::TIME:
    XAxisCalc =
        std::make_unique<DelayLineAmpPosCalc>(ReadoutSettings.EventTimeoutNS);
    break;
  case AdcSettings::PositionSensingType::CONST: // Fall through
  default:
    XAxisCalc = std::make_unique<ConstDelayLinePosition>();
    break;
  }
  XAxisCalc->setCalibrationValues(ReadoutSettings.XAxisCalibOffset,
                                  ReadoutSettings.XAxisCalibSlope);

  switch (ReadoutSettings.YAxis) {
  case AdcSettings::PositionSensingType::AMPLITUDE:
    YAxisCalc =
        std::make_unique<DelayLineAmpPosCalc>(ReadoutSettings.EventTimeoutNS);
    break;
  case AdcSettings::PositionSensingType::TIME:
    YAxisCalc =
        std::make_unique<DelayLineAmpPosCalc>(ReadoutSettings.EventTimeoutNS);
    break;
  case AdcSettings::PositionSensingType::CONST: // Fall through
  default:
    YAxisCalc = std::make_unique<ConstDelayLinePosition>();
    break;
  }
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
  using RoleList = std::tuple<DelayLinePosCalcInterface *,
                              DelayLinePosCalcInterface::ChannelRole,
                              std::function<bool(DelayLinePosCalcInterface *)>>;
  std::multimap<AdcSettings::ChannelRole, RoleList> ChannelRoleMap{
      {AdcSettings::ChannelRole::AMPLITUDE_X_AXIS_1,
       {dynamic_cast<DelayLinePosCalcInterface *>(XAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::FIRST, IsAmpCalc}},
      {AdcSettings::ChannelRole::AMPLITUDE_X_AXIS_2,
       {dynamic_cast<DelayLinePosCalcInterface *>(XAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::SECOND, IsAmpCalc}},
      {AdcSettings::ChannelRole::AMPLITUDE_Y_AXIS_1,
       {dynamic_cast<DelayLinePosCalcInterface *>(YAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::FIRST, IsAmpCalc}},
      {AdcSettings::ChannelRole::AMPLITUDE_Y_AXIS_2,
       {dynamic_cast<DelayLinePosCalcInterface *>(YAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::SECOND, IsAmpCalc}},

      {AdcSettings::ChannelRole::TIME_X_AXIS_1,
       {dynamic_cast<DelayLinePosCalcInterface *>(XAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::FIRST, IsTimeCalc}},
      {AdcSettings::ChannelRole::TIME_X_AXIS_2,
       {dynamic_cast<DelayLinePosCalcInterface *>(XAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::SECOND, IsTimeCalc}},
      {AdcSettings::ChannelRole::TIME_Y_AXIS_1,
       {dynamic_cast<DelayLinePosCalcInterface *>(YAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::FIRST, IsTimeCalc}},
      {AdcSettings::ChannelRole::TIME_Y_AXIS_2,
       {dynamic_cast<DelayLinePosCalcInterface *>(YAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::SECOND, IsTimeCalc}},

      {AdcSettings::ChannelRole::REFERENCE_TIME,
       {dynamic_cast<DelayLinePosCalcInterface *>(YAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::REFERENCE, IsRefCalc}},
      {AdcSettings::ChannelRole::REFERENCE_TIME,
       {dynamic_cast<DelayLinePosCalcInterface *>(XAxisCalc.get()),
        DelayLinePosCalcInterface::ChannelRole::REFERENCE, IsRefCalc}},
  };

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
  DoChannelRoleMapping({0, 0}, ReadoutSettings.ADC1Channel1);
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
