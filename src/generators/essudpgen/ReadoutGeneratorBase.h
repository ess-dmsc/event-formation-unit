// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator base class of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/functiongenerators/DistributionGenerator.h>
#include <common/readout/ess/Parser.h>
#include <common/system/Socket.h>
#include <common/testutils/DataFuzzer.h>
#include <common/time/ESSTime.h>
#include <common/types/DetectorType.h>

#include <CLI/CLI.hpp>

#include <cstdint>

///
/// \class ReadoutGeneratorBase
/// \brief Base class for UDP data generator for ESS readout data.
///
class ReadoutGeneratorBase {
public:
  ///
  /// \struct GeneratorSettings
  /// \brief Struct that holds the generator settings.
  ///
  // clang-format off
  struct GeneratorSettings {
    bool Tof{false};                                        ///< Generate nicely distributed tof data
    bool Debug{false};                                      ///< Print debug info

    uint32_t NFibers{2};                                    ///< Number of fibers
    uint32_t FiberMask{0x00ffffff};                         ///< Mask out unused fibers

    DetectorType Detector{DetectorType::RESERVED};          ///< Data type as specified in the ESS Readout ICD
    std::string IpAddress{"127.0.0.1"};                     ///< IP address for UDP transmission
    uint16_t UDPPort{9000};                                 ///< UDP port for transmission
    uint64_t NumberOfPackets{0};                            ///< Number of packets to transmit (0 means all packets)
    uint32_t NumReadouts{370};                              ///< Number of VMM readouts in the UDP packet
    uint32_t TicksBtwReadouts{10};                          ///< Ticks between readouts
    uint32_t TicksBtwEvents{3 * 88};                        ///< Ticks between events (88 ticks ~1us)
    uint64_t SpeedThrottle{0};                              ///< Speed throttle for transmission
    uint64_t PktThrottle{0};                                ///< Packet throttle for transmission

    /// \todo This should be the default mode and obsolete pe packet generation
    uint16_t Frequency{0};                ///< Frequency of time updates for each packet

    uint8_t headerVersion{1};             ///< Header version
    bool Loop{false};                     ///< Flag to keep looping the same file forever
    bool Randomise{false};                ///< Flag to randomise header and data
    uint32_t KernelTxBufferSize{1000000}; ///< Kernel transmit buffer size
  } Settings;
  // clang-format on

  ///
  /// \brief Constructor for ReadoutGeneratorBase.
  /// \param detectorType The type of detector.
  ///
  ReadoutGeneratorBase(DetectorType detectorType);

  /// \brief Destructor for ReadoutGeneratorBase.
  virtual ~ReadoutGeneratorBase() = default;

  ///
  /// \brief Creates a packet ready for UDP transmission.
  /// \return The type of the packet.
  ///
  uint16_t makePacket();

  ///
  /// \brief Sets the readout data size.
  /// \param ReadoutSize The size of the readout data.
  ///
  void setReadoutDataSize(uint8_t ReadoutSize);

  ///
  /// \brief Process command line arguments, update settings.
  /// Exit application if -h defined or in case of parsing error.
  ///
  /// \param argc The number of command-line arguments.
  /// \param argv The command-line arguments.
  ///
  void argParse(int argc, char *argv[]);

  ///
  /// \brief Sets up buffers, socket, etc.
  ///
  void main();

  ///
  /// \brief Start the transmission loop for the generator.
  ///
  void transmitLoop();

  static constexpr int BufferSize{8972}; ///< Size of the buffer
  uint8_t Buffer[BufferSize];            ///< Buffer for the packet

protected:
  CLI::App app{"UDP data generator for ESS readout data"};

  ///
  /// \brief Generates the header of the packet.
  ///
  void generateHeader();

  ///
  /// \brief Fills out the specified buffer with readouts.
  ///
  virtual void generateData() = 0;

  ///
  /// \brief Finishes the packet by incrementing the sequence number and
  /// performing fuzzing.
  ///
  void finishPacket();

  ///
  /// \brief Increments the readout time with ticks between readouts according
  /// to the settings.
  ///
  inline void addTicksBtwReadoutsToReadoutTime() {
    readoutTime += Settings.TicksBtwReadouts;
  }

  ///
  /// \brief Resets the readout time to the next pulse time.
  ///
  inline void resetReadoutToPulseTime() { readoutTime = getNextPulseTime(); }

  ///
  /// \brief Increments the readout time with ticks between events according to
  /// the settings.
  ///
  inline void addTickBtwEventsToReadoutTime() {
    readoutTime += Settings.TicksBtwEvents;
  }

  ///
  /// \brief Get a tuple containing the high and the low readout time. If the
  /// --ToF option is set the value will include a time of flight distribution
  /// calculated in DistributionGenerator
  /// \return Readout time pair [high, low].
  ///
  virtual std::pair<uint32_t, uint32_t> getReadOutTimes();

  ///
  /// \brief Gets the value of readoutTimeHigh.
  /// \return The value of readoutTimeHigh.
  ///
  inline uint32_t getReadoutTimeHigh() const {
    return readoutTime.getTimeHigh();
  }

  ///
  /// \brief Gets the value of readoutTimeLow.
  /// \return The value of readoutTimeLow.
  ///
  inline uint32_t getReadoutTimeLow() const { return readoutTime.getTimeLow(); }

  ///
  /// \brief Gets the value of readoutTime in nanoseconds.
  /// \return The value of readoutTime in nanoseconds.
  ///
  inline esstime::TimeDurationNano getReadoutTimeNs() const {
    return readoutTime.toNS();
  }

  ///
  /// \brief Gets the value of pulseTime in nanoseconds.
  /// \return The value of pulseTime in nanoseconds.
  ///
  inline esstime::TimeDurationNano getPulseTimeNs() const {
    return pulseTime.toNS();
  }

  ///
  /// \brief Gets the value of prevPulseTime in nanoseconds.
  /// \return The value of prevPulseTime in nanoseconds.
  ///
  inline esstime::TimeDurationNano getPrevPulseTimeNs() const {
    return prevPulseTime.toNS();
  }

  ///
  /// \brief Performs the next pulse time calculation with ESSTime and returns
  /// the next pulse time in nanoseconds.
  /// \return The next pulse time in nanoseconds.
  ///
  inline esstime::TimeDurationNano getNextPulseTimeNs() const {
    esstime::ESSTime nextPulseTime = pulseTime;
    nextPulseTime += pulseFrequencyNs;
    return nextPulseTime.toNS();
  }

  ///
  /// \brief Get a copy of to the pulse time
  /// \return A copy of the pulse time object
  ///
  inline esstime::ESSTime getPulseTime() const { return pulseTime; }

  ///
  /// \brief Performs the next pulse time calculation with ESSTime and returns
  /// the next pulse time.
  /// \return The next pulse time.
  ///
  inline esstime::ESSTime getNextPulseTime() const {
    esstime::ESSTime nextPulseTime = pulseTime;
    nextPulseTime += pulseFrequencyNs;
    return nextPulseTime;
  }

  // clang-format off
  static constexpr int MAX_TIME_DRIFT_NS{20}; ///< Maximum allowed pulse time drift
  const uint32_t TimeLowOffset{20000};        ///< Time offset for readout generation (ticks)
  const uint32_t PrevTimeLowOffset{10000};    ///< Previous time offset for readout generation (ticks)
  uint8_t ReadoutDataSize{0};                 ///< Size of the readout data
  uint16_t NumberOfReadouts{0};               ///< Number of readouts
  uint64_t Packets{0};                        ///< Number of packets
  uint32_t SeqNum{0};                         ///< Sequence number
  uint16_t DataSize{0};                       ///< Number of data bytes in packet
  uint8_t HeaderSize{0};                      ///< Size of the header
  // clang-format on

  DataFuzzer Fuzzer; ///< Data fuzzer
  UDPTransmitter *DataSource{nullptr}; ///< Data source

private:
  ESSReadout::Parser::HeaderVersion headerVersion{
      ESSReadout::Parser::HeaderVersion::V0}; ///< Header version

  // clang-format off
  esstime::ESSTime pulseTime;                    ///< Pulse time
  esstime::ESSTime prevPulseTime;                ///< Previous pulse time
  esstime::ESSTime  readoutTime;                 ///< Readout time
  esstime::TimeDurationNano pulseFrequencyNs{0}; ///< Pulse frequency in nanoseconds
  // clang-format on

  /// \brief For TOF distribution calculations
  /// TofDist could be calculated from default values in Settings struct
  /// by setting Frequency to 14
  DistributionGenerator timeOffFlightDist{ 1000.0/14 };
  float TicksPerMs{  esstime::ESSTime::ESSClockFreqHz/1000.0 };
};
// GCOVR_EXCL_STOP
