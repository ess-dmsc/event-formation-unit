// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of DREAM
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <dream/readout/DataParser.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_WAR

namespace Dream {

constexpr unsigned int DreamReadoutSize{sizeof(DataParser::DreamReadout)};

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
      Stats.ErrorDataHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    auto Data = (DreamReadout *)((char *)DataPtr);

    ///\todo clarify distinction between logical and physical rings
    // for now just divide by two
    Data->RingId = Data->RingId / 2;

    if (BytesLeft < Data->DataLength) {
      XTRACE(DATA, WAR, "Data size mismatch, header says %u got %d",
             Data->DataLength, BytesLeft);
      Stats.ErrorDataHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    if (Data->RingId > MaxRingId or Data->FENId > MaxFENId) {
      XTRACE(DATA, WAR, "Invalid RingId (%u) or FENId (%u)", Data->RingId,
             Data->FENId);
      Stats.ErrorDataHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB, "Ring %u, FEN %u, Length %u", Data->RingId, Data->FENId,
           Data->DataLength);
    Stats.DataHeaders++;

    if (Data->DataLength != DreamReadoutSize) {
      XTRACE(DATA, WAR, "Invalid data length %u, expected %u", Data->DataLength,
             DreamReadoutSize);
      Stats.ErrorDataHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB, "ring %u, fen %u, time: 0x%08x %08x, OM %3u ,"
           "Cathode 0x%3u Anode 0x%3u",
           Data->RingId, Data->FENId, Data->TimeHigh, Data->TimeLow,
           Data->OM, Data->Cathode, Data->Anode);

    ParsedReadouts++;
    Stats.Readouts++;

    Result.push_back(*Data);
    BytesLeft -= Data->DataLength;
    DataPtr += Data->DataLength;
  }

  return ParsedReadouts;
}
} // namespace Loki
