// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Caen Modules
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <caen/readout/DataParser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

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
      Stats.ErrorDataHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    auto Data = (CaenReadout *)((char *)DataPtr);

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

    if (Data->DataLength != CaenReadoutSize) {
      XTRACE(DATA, WAR, "Invalid data length %u, expected %u", Data->DataLength,
             CaenReadoutSize);
      Stats.ErrorDataHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB,
           "ring %u, fen %u, t(%11u,%11u) SeqNo %6u TubeId %3u , A "
           "0x%04x B "
           "0x%04x C 0x%04x D 0x%04x",
           Data->RingId, Data->FENId, Data->TimeHigh, Data->TimeLow,
           Data->DataSeqNum, Data->TubeId, Data->AmpA, Data->AmpB, Data->AmpC,
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
