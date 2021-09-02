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

#include <modules/generators/DataFuzzer.h>
#include <readout/vmm3/VMM3Parser.h>

class ReadoutGenerator {
public:
  ReadoutGenerator() {}

  /// \brief Fill out specified buffer with VMM3 readouts
  /// \param Buffer pointer to the buffer to be filled out with packet data
  /// \param MaxSize Maximum size of generated packet
  /// \param Randomise whether to randomize (fuzz) some of the data
  /// \param Type Data type as specified in the ESS Readout ICD
  /// \param SeqNum sequence number
  /// \param Rings number if rings in use
  /// \param NumReadouts number of VMM readouts in the UDP packet
  uint16_t vmm3ReadoutDataGen(
    uint8_t *Buffer, uint16_t MaxSize, bool Randomise,
    uint8_t Type, uint32_t SeqNum, uint8_t Rings, uint16_t NumReadouts);

private:

  const uint16_t HeaderSize = sizeof(ReadoutParser::PacketHeaderV0);
  const uint16_t VMM3DataSize = sizeof(VMM3Parser::VMM3Data);

  DataFuzzer Fuzzer;
};
// GCOVR_EXCL_STOP
