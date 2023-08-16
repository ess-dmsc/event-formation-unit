// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of TREX
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <trex/readout/DataParser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Trex {

constexpr unsigned int DataHeaderSize{sizeof(ESSReadout::Parser::DataHeader)};
constexpr unsigned int TREXReadoutSize{sizeof(DataParser::TREXReadout)};

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
      return ParsedReadouts;
    }

    auto DataHdrPtr = (ESSReadout::Parser::DataHeader *)DataPtr;

    if (BytesLeft < DataHdrPtr->DataLength) {
      XTRACE(DATA, WAR, "Data size mismatch, header says %u got %d",
             DataHdrPtr->DataLength, BytesLeft);
      Stats.ErrorDataHeaders++;
      return ParsedReadouts;
    }

    if (DataHdrPtr->FiberId > MaxFiberId or DataHdrPtr->FENId > MaxFENId) {
      XTRACE(DATA, WAR, "Invalid FiberId (%u) or FENId (%u)",
             DataHdrPtr->FiberId, DataHdrPtr->FENId);
      Stats.ErrorDataHeaders++;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB, "Fiber %u, FEN %u, Length %u", DataHdrPtr->FiberId,
           DataHdrPtr->FENId, DataHdrPtr->DataLength);
    Stats.DataHeaders++;

    if (DataHdrPtr->DataLength != DataHeaderSize + TREXReadoutSize) {
      XTRACE(DATA, WAR, "Invalid data length %u, expected %u",
             DataHdrPtr->DataLength, DataHeaderSize + TREXReadoutSize);
      Stats.ErrorDataHeaders++;
      return ParsedReadouts;
    }

    ParsedData CurrentDataSection;
    CurrentDataSection.FiberId = DataHdrPtr->FiberId;
    CurrentDataSection.FENId = DataHdrPtr->FENId;

    auto Data = (TREXReadout *)((char *)DataHdrPtr + DataHeaderSize);
    XTRACE(DATA, DEB,
           "fiber %u, fen %u, t(%11u,%11u) SeqNo %6u Group %3u , A "
           "0x%04x B "
           "0x%04x C 0x%04x D 0x%04x",
           DataHdrPtr->FiberId, DataHdrPtr->FENId, Data->TimeHigh,
           Data->TimeLow, Data->DataSeqNum, Data->Group, Data->AmpA, Data->AmpB,
           Data->AmpC, Data->AmpD);

    CurrentDataSection.Data = *Data;
    ParsedReadouts++;
    Stats.Readouts++;

    Result.push_back(CurrentDataSection);
    BytesLeft -= DataHdrPtr->DataLength;
    DataPtr += DataHdrPtr->DataLength;
  }

  return ParsedReadouts;
}
} // namespace Trex
