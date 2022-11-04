// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of LoKI
//===----------------------------------------------------------------------===//

#include <miracles/readout/DataParser.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Miracles {

constexpr unsigned int MiraclesReadoutSize{sizeof(DataParser::MiraclesReadout)};

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

    auto Data = (MiraclesReadout *)((char *)DataPtr);

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

    Stats.DataHeaders++;

    if (Data->DataLength != MiraclesReadoutSize) {
      XTRACE(DATA, WAR, "Invalid data length %u, expected %u", Data->DataLength,
             MiraclesReadoutSize);
      Stats.ErrorDataHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB,
           "Ring %u, FEN %u, t(%11u,%11u) flags %02x, TubeId %3u, "
           "A 0x%2x B 0x%2x",
           Data->RingId, Data->FENId, Data->TimeHigh, Data->TimeLow,
           Data->Flags, Data->TubeId, (uint16_t)Data->AmpA,
           (uint16_t)Data->AmpB);

    ParsedReadouts++;
    Stats.Readouts++;

    Result.push_back(*Data);
    BytesLeft -= Data->DataLength;
    DataPtr += Data->DataLength;
  }

  return ParsedReadouts;
}
} // namespace Miracles
