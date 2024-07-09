// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of DREAM
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <dream/readout/DataParser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

constexpr unsigned int CDTReadoutSize{sizeof(DataParser::CDTReadout)};

// Assume we start after the PacketHeader
int DataParser::parse(const char *Buffer, unsigned int Size) {
  Result.clear();
  unsigned int ParsedReadouts = 0;

  unsigned int BytesLeft = Size;
  char *DataPtr = (char *)Buffer;

  while (BytesLeft) {
    // Parse Data Header
    if (BytesLeft < sizeof(ESSReadout::Parser::DataHeader)) {
      XTRACE(DATA, WAR, "Not enough data left for header: %u", BytesLeft);
      Stats.BufferErrors++;
      return ParsedReadouts;
    }

    auto Data = (CDTReadout *)((char *)DataPtr);

    if (BytesLeft < Data->DataLength) {
      XTRACE(DATA, WAR, "Data size mismatch, header says %u got %d",
             Data->DataLength, BytesLeft);
      Stats.BufferErrors++;
      return ParsedReadouts;
    }

    if (Data->FiberId > MaxFiberId) {
      XTRACE(DATA, WAR, "Invalid FiberId (%u)", Data->FiberId);
      Stats.FiberErrors++;
      return ParsedReadouts;
    }

    if (Data->FENId > MaxFENId) {
      XTRACE(DATA, WAR, "FENId (%u)", Data->FENId);
      Stats.FENErrors++;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB, "Fiber %u, FEN %u, Length %u", Data->FiberId, Data->FENId,
           Data->DataLength);
    Stats.DataHeaders++;

    if (Data->DataLength != CDTReadoutSize) {
      XTRACE(DATA, WAR, "Invalid data length %u, expected %u", Data->DataLength,
             CDTReadoutSize);
      Stats.DataLenErrors++;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB,
           "fiber %u, fen %u, time: 0x%08x %08x, OM %3u ,"
           "Cathode 0x%3u Anode 0x%3u",
           Data->FiberId, Data->FENId, Data->TimeHigh, Data->TimeLow, Data->OM,
           Data->Cathode, Data->Anode);

    ParsedReadouts++;
    Stats.Readouts++;

    Result.push_back(*Data);
    BytesLeft -= Data->DataLength;
    DataPtr += Data->DataLength;
  }

  return ParsedReadouts;
}
} // namespace Dream
