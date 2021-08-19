// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3a readouts with variable number
/// of readouts
//===----------------------------------------------------------------------===//

#pragma once

#include <readout/vmm3/VMM3Parser.h>
#include <modules/generators/DataFuzzer.h>

class ReadoutGenerator {
public:
  ReadoutGenerator() {}

  /// \brief Fill out specified buffer with LoKI readouts
  uint16_t vmm3ReadoutDataGen(uint8_t Type, bool Randomise, uint16_t NumReadouts,
                              uint8_t Rings,
                              uint8_t *Buffer, uint16_t MaxSize,
                              uint32_t SeqNum);

private:

  const uint16_t HeaderSize = sizeof(ReadoutParser::PacketHeaderV0);
  const uint16_t VMM3DataSize = sizeof(VMM3Parser::VMM3Data);

  DataFuzzer Fuzzer;
};
