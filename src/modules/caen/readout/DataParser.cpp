// Copyright (C) 2019 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Caen Modules
//===----------------------------------------------------------------------===//

#include <caen/readout/DataParser.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

constexpr unsigned int CaenReadoutSize{sizeof(DataParser::CaenReadout)};

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
      Stats.DataHeaderSizeErrors++;
      return ParsedReadouts;
    }

    auto Data = (CaenReadout *)((char *)DataPtr);

    if (BytesLeft < Data->DataLength) {
      XTRACE(DATA, WAR, "Data size mismatch, header says %u got %d",
             Data->DataLength, BytesLeft);
      Stats.DataLenMismatch++;
      return ParsedReadouts;
    }

    if (Data->FiberId > MaxFiberId or Data->FENId > MaxFENId) {
      XTRACE(DATA, WAR, "Invalid FiberId (%u) or FENId (%u)", Data->FiberId,
             Data->FENId);
      Stats.RingFenErrors++;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB, "Fiber %u, FEN %u, Length %u", Data->FiberId, Data->FENId,
           Data->DataLength);
    Stats.DataHeaders++;

    if (Data->DataLength != CaenReadoutSize) {
      XTRACE(DATA, WAR, "Invalid data length %u, expected %u", Data->DataLength,
             CaenReadoutSize);
      Stats.DataLenInvalid++;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB,
           "fiber %u, fen %u, t(%11u,%11u) SeqNo %6u Group %3u , A "
           "0x%04x B "
           "0x%04x C 0x%04x D 0x%04x",
           Data->FiberId, Data->FENId, Data->TimeHigh, Data->TimeLow,
           Data->DataSeqNum, Data->Group, Data->AmpA, Data->AmpB, Data->AmpC,
           Data->AmpD);

    ParsedReadouts++;
    Stats.Readouts++;

    Result.push_back(*Data);
    BytesLeft -= Data->DataLength;
    DataPtr += Data->DataLength;
  }

  return ParsedReadouts;
}
} // namespace Caen
