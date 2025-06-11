// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Holder for readout data (DTO) objects used for timepix3 module.
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <cstdint>

#define PIXEL_MAX_TIMESTAMP_NS 26843545600

#define TDC_CLOCK_BIN_NS 3.125
#define TDC_FINE_CLOCK_BIN_NS 0.26

namespace timepixDTO {

///
/// \brief Structure representing the global timestamp of an ESS event.
///
/// This structure contains two fields: pulseTimeInEpochNs and
/// tdcClockInPixelTime, representing the pulse time in epoch nanoseconds and
/// the TDC clock in pixel time, respectively.
///
struct ESSGlobalTimeStamp {
  const uint64_t pulseTimeInEpochNs;  // < Pulse time in epoch nanoseconds.
  const uint64_t tdcClockInPixelTime; // < TDC clock in pixel time.

  ///
  /// \brief Constructor for ESSGlobalTimeStamp.
  ///
  /// \param pulseTimeInEpochNs The pulse time in epoch nanoseconds.
  /// \param tdcClockInPixelTime The TDC clock in pixel time.
  ///
  ESSGlobalTimeStamp(uint64_t pulseTimeInEpochNs,
                     uint64_t tdcClockInPixelTime)
      : pulseTimeInEpochNs(pulseTimeInEpochNs),
        tdcClockInPixelTime(tdcClockInPixelTime) {}

  ///
  /// \brief Equality comparison operator for ESSGlobalTimeStamp.
  ///
  /// \param other The ESSGlobalTimeStamp object to compare with.
  /// \return true if the two objects are equal, false otherwise.
  ///
  bool operator==(const ESSGlobalTimeStamp &other) const {
    return pulseTimeInEpochNs == other.pulseTimeInEpochNs &&
           tdcClockInPixelTime == other.tdcClockInPixelTime;
  }
};

///
/// \brief Represents an EVR data event.
///
/// This struct contains information about an EVR data event, including the
/// counter value, the pulse time in epoch nanoseconds, and the arrival
/// timestamp.
///
struct EVRDataEvent {
  const uint32_t counter;            // < The counter value.
  const uint64_t pulseTimeInEpochNs; // < The pulse time in epoch nanoseconds.
  const std::chrono::high_resolution_clock::time_point
      arrivalTimestamp; // < The arrival timestamp.

  ///
  /// \brief Constructs an EVRDataEvent object.
  ///
  /// \param counter The counter value.
  /// \param pulseTimeSeconds The pulse time in seconds.
  /// \param pulseTimeNanoSeconds The pulse time in nanoseconds.
  ///
  EVRDataEvent(const uint32_t counter, const uint32_t pulseTimeSeconds,
               const uint32_t pulseTimeNanoSeconds)
      : counter(counter),
        pulseTimeInEpochNs(pulseTimeSeconds * 1e9 + pulseTimeNanoSeconds),
        arrivalTimestamp(std::chrono::high_resolution_clock::now()) {}

  ///
  /// \brief Equality comparison operator.
  ///
  /// \param other The other EVRDataEvent object to compare with.
  /// \return true if the objects are equal, false otherwise.
  ///
  bool operator==(const EVRDataEvent &other) const {
    return counter == other.counter &&
           pulseTimeInEpochNs == other.pulseTimeInEpochNs &&
           arrivalTimestamp == other.arrivalTimestamp;
  }
};

///
/// \brief Structure representing a TDC data event.
///
struct TDCDataEvent {
  const uint16_t counter;             // < The trigger counter value.
  const uint64_t tdcTimeStamp;        // < The TDC timestamp.
  const uint8_t pixelClockQuarter;    // < The pixel clock quarter value.
  const uint64_t tdcTimeInPixelClock; // < The TDC time in pixel clock.
  const std::chrono::high_resolution_clock::time_point
      arrivalTimestamp; // < The arrival timestamp.

  ///
  /// \brief Constructor for TDCDataEvent.
  /// \param triggerCounter The trigger counter value.
  /// \param timestamp The TDC timestamp.
  /// \param stamp The stamp value.
  ///
  TDCDataEvent(uint16_t triggerCounter, uint64_t timestamp, uint8_t stamp)
      : counter(triggerCounter), tdcTimeStamp(TDC_CLOCK_BIN_NS * timestamp +
                                              TDC_FINE_CLOCK_BIN_NS * stamp),
        pixelClockQuarter(uint8_t(tdcTimeStamp / PIXEL_MAX_TIMESTAMP_NS)),
        tdcTimeInPixelClock(tdcTimeStamp -
                            PIXEL_MAX_TIMESTAMP_NS * pixelClockQuarter),
        arrivalTimestamp(std::chrono::high_resolution_clock::now()) {}

  ///
  /// \brief Equality comparison operator.
  /// \param other The other TDCDataEvent to compare with.
  /// \return True if the TDCDataEvents are equal, false otherwise.
  ///
  bool operator==(const TDCDataEvent &other) const {
    return counter == other.counter && tdcTimeStamp == other.tdcTimeStamp &&
           pixelClockQuarter == other.pixelClockQuarter &&
           tdcTimeInPixelClock == other.tdcTimeInPixelClock &&
           arrivalTimestamp == other.arrivalTimestamp;
  }

private:
  ///
  /// \brief Converts TDC time to pixel time.
  /// \param tdcTimeToConvert The TDC time to convert.
  /// \return The converted pixel time.
  ///
  inline uint64_t
  convertTdcTimeToPixelTime(const uint64_t tdcTimeToConvert) const {
    return tdcTimeToConvert -
           (PIXEL_MAX_TIMESTAMP_NS *
            uint16_t(tdcTimeToConvert / PIXEL_MAX_TIMESTAMP_NS));
  }
};

} // namespace timepixDTO

namespace timepixReadout {

///
/// \brief Structure representing the EVR readout data.
///
/// This structure holds the EVR readout data, including various fields such as
/// type, counter, pulse time, and previous pulse time. It also provides an
/// equality comparison operator for comparing two EVRReadout objects.
///
struct EVRReadout {
  const uint8_t type;                  // < The type of the EVR readout.
  const uint8_t unused;                // < Unused field.
  const uint16_t unused2;              // < Unused field.
  const uint32_t counter;              // < The counter value.
  const uint32_t pulseTimeSeconds;     // < The pulse time in seconds.
  const uint32_t pulseTimeNanoSeconds; // < The pulse time in nanoseconds.
  const uint32_t prevPulseTimeSeconds; // < The previous pulse time in seconds.
  const uint32_t
      prevPulseTimeNanoSeconds; // < The previous pulse time in nanoseconds.

  ///
  /// \brief Constructor for EVRReadout.
  ///
  /// \param type The type of the EVR readout.
  /// \param unused Unused field.
  /// \param unused2 Unused field.
  /// \param counter The counter value.
  /// \param pulseTimeSeconds The pulse time in seconds.
  /// \param pulseTimeNanoSeconds The pulse time in nanoseconds.
  /// \param prevPulseTimeSeconds The previous pulse time in seconds.
  /// \param prevPulseTimeNanoSeconds The previous pulse time in nanoseconds.
  ///
  EVRReadout(uint8_t type, uint8_t unused, uint16_t unused2, uint32_t counter,
             uint32_t pulseTimeSeconds, uint32_t pulseTimeNanoSeconds,
             uint32_t prevPulseTimeSeconds, uint32_t prevPulseTimeNanoSeconds)
      : type(type), unused(unused), unused2(unused2), counter(counter),
        pulseTimeSeconds(pulseTimeSeconds),
        pulseTimeNanoSeconds(pulseTimeNanoSeconds),
        prevPulseTimeSeconds(prevPulseTimeSeconds),
        prevPulseTimeNanoSeconds(prevPulseTimeNanoSeconds) {}

  ///
  /// \brief Equality comparison operator for EVRReadout.
  ///
  /// \param other The EVRReadout object to compare with.
  /// \return true if the EVRReadout objects are equal, false otherwise.
  ///
  bool operator==(const EVRReadout &other) const {
    return type == other.type && unused == other.unused &&
           unused2 == other.unused2 && counter == other.counter &&
           pulseTimeSeconds == other.pulseTimeSeconds &&
           pulseTimeNanoSeconds == other.pulseTimeNanoSeconds &&
           prevPulseTimeSeconds == other.prevPulseTimeSeconds &&
           prevPulseTimeNanoSeconds == other.prevPulseTimeNanoSeconds;
  }
} __attribute__((__packed__));

///
/// \brief Represents a TDC (Time-to-Digital Converter) readout.
///
/// This struct stores information about a TDC readout, including the type,
/// counter value, timestamp, and stamp. It also provides an equality comparison
/// operator for comparing two TDCReadout objects.
///
struct TDCReadout {
  const uint8_t type;       // < The type of the TDC readout.
  const uint16_t counter;   // < The counter value of the TDC readout.
  const uint64_t timestamp; // < The timestamp of the TDC readout.
  const uint8_t stamp;      // < The stamp of the TDC readout.

  ///
  /// \brief Constructs a TDCReadout object with the specified parameters.
  ///
  /// \param type The type of the TDC readout.
  /// \param counter The counter value of the TDC readout.
  /// \param timestamp The timestamp of the TDC readout.
  /// \param stamp The stamp of the TDC readout.
  ///
  TDCReadout(uint8_t type, uint16_t counter, uint64_t timestamp, uint8_t stamp)
      : type(type), counter(counter), timestamp(timestamp), stamp(stamp) {}

  ///
  /// \brief Equality comparison operator for comparing two TDCReadout objects.
  ///
  /// \param other The TDCReadout object to compare with.
  /// \return true if the TDCReadout objects are equal, false otherwise.
  ///
  bool operator==(const TDCReadout &other) const {
    return type == other.type && counter == other.counter &&
           timestamp == other.timestamp && stamp == other.stamp;
  }
};

///
/// \brief Represents the readout data of a single pixel in a Timepix sensor.
///
struct PixelReadout {
  const uint16_t dCol;      // < The digital column of the pixel.
  const uint16_t sPix;      // < The sub pixel index of the pixel.
  const uint8_t pix;        // < The pixel index within the sub pixel.
  const uint16_t toa;       // < The Time-of-Arrival value of the pixel.
  const uint16_t ToT;       // < The Time-over-Threshold value of the pixel.
  const uint8_t fToA;       // < The fine Time-of-Arrival value of the pixel.
  const uint32_t spidrTime; // < The SPIDR timestamp of the pixel.

  ///
  /// \brief Constructs a PixelReadout object with the specified parameters.
  ///
  /// \param dCol The digital column of the pixel.
  /// \param sPix The sub pixel index of the pixel.
  /// \param pix The pixel index within the sub pixel.
  /// \param toa The Time-of-Arrival value of the pixel.
  /// \param ToT The Time-over-Threshold value of the pixel.
  /// \param fToA The fine Time-of-Arrival value of the pixel.
  /// \param spidrTime The SPIDR timestamp of the pixel.
  ///
  PixelReadout(uint16_t dCol, uint16_t sPix, uint8_t pix, uint16_t toa,
               uint16_t ToT, uint8_t fToA, uint32_t spidrTime)
      : dCol(dCol), sPix(sPix), pix(pix), toa(toa), ToT(ToT), fToA(fToA),
        spidrTime(spidrTime) {}

  ///
  /// \brief Compares two PixelReadout objects for equality.
  ///
  /// \param other The other PixelReadout object to compare.
  /// \return true if the objects are equal, false otherwise.
  ///
  bool operator==(const PixelReadout &other) const {
    return dCol == other.dCol && sPix == other.sPix && pix == other.pix &&
           ToT == other.ToT && fToA == other.fToA && toa == other.toa &&
           spidrTime == other.spidrTime;
  }
};

} // namespace timepixReadout