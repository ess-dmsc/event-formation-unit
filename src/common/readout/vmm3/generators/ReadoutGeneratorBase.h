// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3a readouts with variable number
/// of readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/readout/vmm3/VMM3Parser.h>
#include <common/testutils/DataFuzzer.h>

class ReadoutGeneratorBase {
public:
  struct GeneratorSettings {
    uint16_t NRings{2};
    uint8_t Type{72}; // Freia (see readout ICD for other instruments)
    /// udp generator generic
    std::string IpAddress{"127.0.0.1"};
    uint16_t UDPPort{9000};
    uint64_t NumberOfPackets{0};     // 0 == all packets
    uint32_t NumReadouts{400};       // # readouts in packet
    uint32_t TicksBtwReadouts{88};   // 88 ticks ~ 1us
    uint32_t TicksBtwEvents{3 * 88}; // 3 * 88 ticks ~ 3us
    uint64_t SpeedThrottle{0};       // 0 is fastest higher is slower
    uint64_t PktThrottle{0};         // 0 is fastest
    bool Loop{false};                // Keep looping the same file forever

    bool Randomise{false}; // Randomise header and data
    // Not yet CLI settings
    uint32_t KernelTxBufferSize{1000000};
  };

  /// \brief Setup buffer and sequence number
  /// \param Buffer pointer to the buffer to be filled out with packet data
  /// \param BufferSize Maximum size of generated packet
  /// \param SeqNum sequence number
  /// \param Randomise whether to randomize (fuzz) some of the data
  ReadoutGeneratorBase(uint8_t *Buffer, uint16_t BufferSize,
                       uint32_t InitialSeqNum, GeneratorSettings &Settings);
  // ReadoutGeneratorBase() = default;

  /// \brief create a packet ready for UDP transmission, calls private methods
  /// \param Type Data type as specified in the ESS Readout ICD
  /// \param NumReadouts number of VMM readouts in the UDP packet
  /// \param Rings number if rings in use
  uint16_t makePacket();

protected:
  /// \brief Generate common readout header
  /// \param Type Data type as specified in the ESS Readout ICD
  /// \param NumReadouts number of VMM readouts in the UDP packet
  void generateHeader();

  /// \brief Fill out specified buffer with VMM3 readouts
  /// \param Rings number if rings in use
  /// \param NumReadouts number of VMM readouts in the UDP packet
  virtual void generateData() = 0;

  /// \brief Increment sequence number and do fuzzing
  void finishPacket();

  const uint16_t HeaderSize = sizeof(ESSReadout::Parser::PacketHeaderV0);
  const uint16_t VMM3DataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);

  GeneratorSettings &Settings;
  // Time offsets for readout generation
  const uint32_t TimeLowOffset{20000};     // ticks
  const uint32_t PrevTimeLowOffset{10000}; // ticks
  // const uint32_t TimeToFirstReadout{1000}; // ticks

  uint8_t *Buffer{nullptr};
  uint16_t BufferSize{0};
  uint32_t SeqNum{0};
  uint32_t TimeHigh{0};
  uint32_t TimeLow{0};
  uint16_t DataSize{0}; // Number of data bytes in packet

  DataFuzzer Fuzzer;
};
// GCOVR_EXCL_STOP
