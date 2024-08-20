// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator base class of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <CLI/CLI.hpp>
#include <common/readout/ess/Parser.h>
#include <common/system/Socket.h>
#include <common/testutils/DataFuzzer.h>
#include <common/time/ESSTime.h>
#include <cstdint>

class ReadoutGeneratorBase {
public:
  struct GeneratorSettings {
    uint16_t NFibers{2};
    uint8_t Type{0}; // Will be determined at compile time, can be overridden
    uint8_t TypeOverride{0}; // to force a specific type field
    /// udp generator generic
    std::string IpAddress{"127.0.0.1"};
    uint16_t UDPPort{9000};
    uint64_t NumberOfPackets{0};     // 0 == all packets
    uint32_t NumReadouts{370};       // # readouts in packet
    uint32_t TicksBtwReadouts{10};   // 88 ticks ~ 1us
    uint32_t TicksBtwEvents{3 * 88}; // 3 * 88 ticks ~ 3us
    uint64_t SpeedThrottle{0};       // 0 is fastest higher is slower
    uint64_t PktThrottle{0};         // 0 is fastest
    /// \todo This should be the default mode and obsoete pe packet generation
    uint16_t Frequency{0};    // 0 is time updates for each packet
    uint8_t headerVersion{1}; // v1 header by default
    bool Loop{false};         // Keep looping the same file forever

    bool Randomise{false}; // Randomise header and data
    // Not yet CLI settings
    uint32_t KernelTxBufferSize{1000000};
  } Settings;

  /// \brief
  ReadoutGeneratorBase(ESSReadout::Parser::DetectorType);

  /// \brief create a packet ready for UDP transmission, calls private methods
  /// \param Type Data type as specified in the ESS Readout ICD
  /// \param NumReadouts number of VMM readouts in the UDP packet
  /// \param Rings number if rings in use
  uint16_t makePacket();

  /// \brief Set the readout data size (required)
  void setReadoutDataSize(uint8_t ReadoutSize) {
    ReadoutDataSize = ReadoutSize;
  }

  /// \brief update Settings based on CLI arguments
  int argParse(int argc, char *argv[]);

  /// \brief setup buffers, socket etc.
  void main();

  /// \brief transmit the specified packets
  void transmitLoop();

  static constexpr int BufferSize{8972};
  uint8_t Buffer[BufferSize];

protected:
  CLI::App app{"UDP data generator for ESS readout data"};

  /// \brief Generate common readout header
  /// \param Type Data type as specified in the ESS Readout ICD
  /// \param NumReadouts number of readouts in the UDP packet
  void generateHeader();

  /// \brief Fill out specified buffer with readouts
  virtual void generateData() = 0;

  /// \brief Increment sequence number and do fuzzing
  void finishPacket();

  /// \brief Increment the readout time with ticks btw. readouts according to
  /// settings
  inline void addTicksBtwReadoutsToReadoutTime() {
    readoutTime += Settings.TicksBtwReadouts;
  }

  /// \brief Reset the readout time to the next pulse time
  inline void resetReadoutToPulseTime() { readoutTime = getNextPulseTime(); }

  /// \brief Increment the readout time with btw. events acording to
  /// settings
  inline void addTickBtwEventsToReadoutTime() {
    readoutTime += Settings.TicksBtwEvents;
  }

  // Get the value of readoutTimeHigh
  uint32_t getReadoutTimeHigh() const { return readoutTime.getTimeHigh(); }

  // Get the value of readoutTimeLow
  uint32_t getReadoutTimeLow() const { return readoutTime.getTimeLow(); }

  // Get the value of readoutTime
  esstime::TimeDurationNano getReadoutTimeNs() const {
    return readoutTime.toNS();
  }

  // Get the value of pulseTime
  esstime::TimeDurationNano getPulseTimeNs() const { return pulseTime.toNS(); }

  // Get the value of prevPulseTime
  esstime::TimeDurationNano getPrevPulseTimeNs() const {
    return prevPulseTime.toNS();
  }

  /// \brief Perform the next pulse time calculation with ESSTime and return the
  /// nanosecond precesion next pulse time, calculated according to ESS clock
  /// ticks
  esstime::TimeDurationNano getNextPulseTimeNs() const {
    esstime::ESSTime nextPulseTime = pulseTime;
    nextPulseTime += pulseFrequencyNs;
    return nextPulseTime.toNS();
  }

  esstime::ESSTime getNextPulseTime() const {
    esstime::ESSTime nextPulseTime = pulseTime;
    nextPulseTime += pulseFrequencyNs;
    return nextPulseTime;
  }

  // Time offsets for readout generation
  const uint32_t TimeLowOffset{20000};     // ticks
  const uint32_t PrevTimeLowOffset{10000}; // ticks
  // const uint32_t TimeToFirstReadout{1000}; // ticks

  uint8_t ReadoutDataSize{0};
  uint16_t numberOfReadouts{0};

  uint64_t Packets{0};
  uint32_t SeqNum{0};
  uint16_t DataSize{0}; // Number of data bytes in packet
  uint8_t HeaderSize{0};

  DataFuzzer Fuzzer;

  UDPTransmitter *DataSource{nullptr};

private:
  ESSReadout::Parser::HeaderVersion headerVersion{
      ESSReadout::Parser::HeaderVersion::V0};

  ESSReadout::ESSTime pulseTime;
  ESSReadout::ESSTime prevPulseTime;
  ESSReadout::ESSTime readoutTime;

  esstime::TimeDurationNano pulseFrequencyNs{0};
};
// GCOVR_EXCL_STOP
