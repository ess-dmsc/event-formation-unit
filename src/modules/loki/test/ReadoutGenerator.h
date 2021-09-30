// Copyright (C) 2019 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial LoKI readouts with variable number
/// of sections and data elements per section.
//===----------------------------------------------------------------------===//

#pragma once

#include <loki/readout/DataParser.h>
#include <modules/generators/DataFuzzer.h>

class ReadoutGenerator {
public:
  ReadoutGenerator() {}

  /// \brief Fill out specified buffer with LoKI readouts
  uint16_t lokiReadoutDataGen(bool Randomise, uint16_t DataSections,
                              uint16_t DataElements, uint8_t Rings,
                              uint8_t *Buffer, uint16_t MaxSize,
                              uint32_t SeqNum);

private:
  static_assert(sizeof(Loki::DataParser::LokiReadout) == 20,
                "Loki data format mismatch");

  const uint16_t HeaderSize = sizeof(ESSReadout::Parser::PacketHeaderV0);
  const uint16_t DataHeaderSize = sizeof(ESSReadout::Parser::DataHeader);
  const uint16_t LokiDataSize = sizeof(Loki::DataParser::LokiReadout);

  DataFuzzer Fuzzer;
};
