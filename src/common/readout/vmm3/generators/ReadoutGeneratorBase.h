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

#include <common/testutils/DataFuzzer.h>
#include <common/readout/vmm3/VMM3Parser.h>

class ReadoutGeneratorBase {
public:
  /// \brief Setup buffer and sequence number
  /// \param Buffer pointer to the buffer to be filled out with packet data
  /// \param BufferSize Maximum size of generated packet
  /// \param SeqNum sequence number
  /// \param Randomise whether to randomize (fuzz) some of the data
  ReadoutGeneratorBase(uint8_t *Buffer, uint16_t BufferSize,
    uint32_t InitialSeqNum, bool Randomise);
  // ReadoutGeneratorBase() = default;

  /// \brief create a packet ready for UDP transmission, calls private methods
  /// \param Type Data type as specified in the ESS Readout ICD
  /// \param NumReadouts number of VMM readouts in the UDP packet
  /// \param Rings number if rings in use
  uint16_t makePacket(
    uint8_t Type, uint16_t NumReadouts,
    uint32_t TicksBtwReadouts, uint32_t TicksBtwEvents);


protected:
  /// \brief Generate common readout header
  /// \param Type Data type as specified in the ESS Readout ICD
  /// \param NumReadouts number of VMM readouts in the UDP packet
  void generateHeader(uint8_t Type, uint16_t NumReadouts);

  /// \brief Fill out specified buffer with VMM3 readouts
  /// \param Rings number if rings in use
  /// \param NumReadouts number of VMM readouts in the UDP packet
  virtual void generateData(uint16_t NumReadouts) = 0;

  /// \brief Increment sequence number and do fuzzing
  void finishPacket();

  const uint16_t HeaderSize = sizeof(ESSReadout::Parser::PacketHeaderV0);
  const uint16_t VMM3DataSize = sizeof(ESSReadout::VMM3Parser::VMM3Data);

  // Time offsets for readout generation
  const uint32_t TimeLowOffset{20000};     // ticks
  const uint32_t PrevTimeLowOffset{10000}; // ticks
  // const uint32_t TimeToFirstReadout{1000}; // ticks
  uint32_t TimeBtwReadout{88};       // ticks ~ 1us
  uint32_t TimeBtwEvents{88 * 3};    // ticks ~ 3us
  uint8_t Rings{4};

  uint8_t * Buffer{nullptr};
  uint16_t BufferSize{0};
  uint32_t SeqNum{0};
  uint32_t TimeHigh{0};
  uint16_t DataSize{0}; // Number of data bytes in packet
  bool Random{false};

  DataFuzzer Fuzzer;
};
// GCOVR_EXCL_STOP
