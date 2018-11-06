/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcSettings.h"
#include "DelayLinePositionCalc.h"
#include "PulseParameters.h"
#include <cstdint>
#include <limits>
#include <map>
#include <memory>

/// \brief The event information extracted from one or several pulses from the
/// ADC system.
struct DelayLineEvent {
  int X;
  int Y;
  int Amplitude;
  std::uint64_t Timestamp;
};

/// \brief Class for setting up position calculation, passing pulses to the
/// correct calculator and extracting valid events.
class DelayLineEventFormation {
public:
  /// \brief Constructor sets up the event formation based on the supplied
  /// readout settings.
  ///
  /// \param[in] ReadoutSettings The settings are used to determine which kind
  /// of pulse processing is done for each axis and which channel belongs to
  /// each axis.
  DelayLineEventFormation(AdcSettings const &ReadoutSettings);

  /// \brief Handle a pulse.
  ///
  /// \param[in] Pulse The ADC pulse information to process.
  void addPulse(PulseParameters const &Pulse);

  /// \brief Does the class have a valid event.
  ///
  /// Should be called after every call to addPulse().
  /// \return true if an event can be extracted from the data supplied using
  /// addPulse(). Returns false otherwise.
  bool hasValidEvent();

  /// \brief Get event extracted from pulse information.
  ///
  /// Will remove existing event data when calling and should thus never be
  /// called again without any calls to addPulse() and hasValidEvent().
  /// \return The fully formed event extracted from the pulse data if
  /// hasValidEvent() is true. Default constructed DelayLineEvent otherwise.
  DelayLineEvent popEvent();

  /// \brief Get the number of processed pulses, i.e. pulses from channels
  /// registered to an axis.
  /// \note Does not guarantee that all the pulses (the return value) were used
  /// in the creation of an event.
  auto getNrOfProcessedPulses() { return ProcessedDelayLinePulses; }
  
  /// \brief Get the number of discarded pulses, i.e. pulses from channels NOT
  /// registered to an axis.
  /// \note Does not guarantee that all the pulses (the return value) were used
  /// in the creation of an event.
  auto getNrOfDiscardedPulses() { return DiscardedDelayLinePulses; }

protected:
  void DoChannelRoleMapping(ChannelID ID, AdcSettings::ChannelRole Role);
  std::int64_t ProcessedDelayLinePulses{0};
  std::int64_t DiscardedDelayLinePulses{0};
  std::unique_ptr<DelayLinePositionInterface> XAxisCalc;
  std::unique_ptr<DelayLinePositionInterface> YAxisCalc;
  std::multimap<ChannelID, DelayLinePositionInterface *> PulseHandlerMap;
};
