/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief The interfaces used to calculate the position of an event.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "ChannelID.h"
#include "PulseParameters.h"
#include <cmath>
#include <cstdint>
#include <limits>
#include <unordered_map>

struct AxisEvent {
  int Position{0};
  int Amplitude{0};
  std::uint64_t Timestamp{0};
};

class DelayLinePositionInterface {
public:
  DelayLinePositionInterface() = default;
  virtual ~DelayLinePositionInterface() = default;
  virtual bool isValid() { return true; }
  virtual void setCalibrationValues(double NewOrigin, double NewSlope) {
    Origin = NewOrigin;
    Slope = NewSlope;
  };

  struct CalibData {
    double Origin;
    double Slope;
  };
  virtual CalibData getCalibrationValues() {
    return {Origin, Slope};
  };

  /// \brief Calculate event information based on available pulse data.
  ///
  /// Will remove existing pulse information on when calling.
  /// \note Only call this function if isValid() has returned true.
  /// \return Event information for current axis.
  virtual AxisEvent popEvent() {
    AxisEvent ReturnValue{getPosition(), getAmplitude(), getTimestamp()};
    reset();
    return ReturnValue;
  }

protected:
  virtual int getPosition() = 0;
  virtual int getAmplitude() = 0;
  virtual std::uint64_t getTimestamp() { return 0; };
  virtual void reset(){};

  int applyCalibration(double RawPosition) {
    return std::lround(RawPosition * Slope + Origin);
  };
  double Origin{0};
  double Slope{1.0};
};

class ConstDelayLinePosition : public DelayLinePositionInterface {
public:
  ConstDelayLinePosition() = default;

protected:
  int getPosition() override;
  int getAmplitude() override { return 0; };
};

class DelayLinePosCalcInterface : public DelayLinePositionInterface {
public:
  enum class ChannelRole { FIRST, SECOND, REFERENCE };
  explicit DelayLinePosCalcInterface(std::uint64_t Timeout);
  /// \brief Add a pulse to the list of available pulses for event processing.
  ///
  /// \param[in] Pulse The pulse to add to list of pulses.
  /// \note Calling this function with pulses that has the same ID will result
  /// in only the latter pulse being stored.
  virtual void addPulse(PulseParameters const &Pulse);
  /// \brief Tie a channel ID to a specific role.
  ///
  /// It is up to children of this class to determine if all required roles are
  /// filled.
  /// \param[in] ID The identifier of a channel.
  /// \param[in] Role The role of that channel.
  virtual void setChannelRole(ChannelID const &ID, ChannelRole const &Role);

protected:
  /// \brief Removes all pulse information submitted through addPuls().
  /// \note Should be called after the a position/timestamp/amplitude has been
  /// extracted in order to prevent double registration of pulses.
  void reset() override;

  int getPosition() override { return 0; }
  /// \brief Verify that a valid event can be constructed from the data.
  ///
  /// Will return false if one or more of the following is true:
  /// - Not all selected (pulse) roles have been filled (using addPulse()).
  /// - There are no (pulse) roles assigned.
  /// - One or more of the timestamps are set to 0.
  /// - The difference between first and last timestamp is greater than the
  /// timeout set in the constructor.
  /// \note Additional checks might be implemented by classes that inherit from
  /// DelayLinePosCalcInterface.
  /// \return true if valid event can be extracted, false otherwise.
  bool isValid() override;

  /// \brief Get the timestamp of the current event data.
  ///
  /// The timestamp returned is that of maximum value of the pulse data if a
  /// reference time pulse is available or for the "FIRST" pulse if not.
  /// \note Is is up to the user of this class to make sure that the event is
  /// valid.
  /// \return Timestamp in nanoseconds since Unix EPOCH.
  std::uint64_t getTimestamp() override;

  std::unordered_map<ChannelID, ChannelRole> RoleMap;
  std::unordered_map<ChannelRole, PulseParameters> PulseData;
  std::uint64_t EventTimeout{0};
};

class DelayLineAmpPosCalc : public DelayLinePosCalcInterface {
public:
  explicit DelayLineAmpPosCalc(std::uint64_t Timeout);

protected:
  /// \brief Calculate position of event along one axis based on the amplitude
  /// of one or two pulses fromt that axis.
  ///
  /// \note You must check that there is a valid event (using isValid()) before
  /// calling this function.
  /// \return The position of the event in the current axis.
  int getPosition() override;

  /// \brief Get the amplitude of an event.
  ///
  /// This class extracts the amplitude for the event from the peak area of the
  /// reference pulse if available.
  /// \note You must check that there is a valid event (using isValid()) before
  /// calling this function.
  /// \return Area of the reference pulse if available. Zero (0) otherwise.
  int getAmplitude() override;

  /// \brief Determine if we can extract a valid event.
  ///
  /// Checks that we have pulses that are the minimum required to extract an
  /// event for the current axis. Also calls
  /// DelayLinePosCalcInterface::isValid().
  /// \return true if we can extract an event, false otherwise.
  bool isValid() override;
};

class DelayLineTimePosCalc : public DelayLinePosCalcInterface {
public:
  explicit DelayLineTimePosCalc(std::uint64_t Timeout);

protected:
  /// \brief Calculate position in detector along one axis based on timestamp of
  /// pulses.
  ///
  /// \note The reference pulse uses the timestamp of the maximum value in the
  /// pulse. Consider changing this to the threshold value.
  /// \note You must check that there is a valid event (using isValid()) before
  /// calling this function.
  /// \return The position of the event in the current axis.
  int getPosition() override;

  /// \brief Get the amplitude of an event.
  ///
  /// This class extracts the amplitude for the event from the peak area of the
  /// reference pulse.
  /// \note You must check that there is a valid event (using isValid()) before
  /// calling this function.
  /// \return Area of the reference pulse.
  int getAmplitude() override;

  /// \brief Determine if we can extract a valid event.
  ///
  /// Checks that we have pulses that are the minimum required to extract an
  /// event for the current axis. Also calls
  /// DelayLinePosCalcInterface::isValid().
  /// \return true if we can extract an event, false otherwise.
  bool isValid() override;
};
