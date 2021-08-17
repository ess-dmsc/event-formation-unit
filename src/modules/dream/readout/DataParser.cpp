// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of DREAM
//===----------------------------------------------------------------------===//

#include <common/Trace.h>
#include <dream/readout/DataParser.h>
#include <readout/common/ReadoutParser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

constexpr unsigned int DataHeaderSize{sizeof(ReadoutParser::DataHeader)};
constexpr unsigned int DreamReadoutSize{sizeof(DataParser::DreamReadout)};

// Assume we start after the PacketHeader
int DataParser::parse(const char *Buffer, unsigned int Size) {
  Result.clear();
  unsigned int ParsedReadouts = 0;

  unsigned int BytesLeft = Size;
  char *DataPtr = (char *)Buffer;

  while (BytesLeft) {
    // Parse Data Header
    if (BytesLeft < sizeof(ReadoutParser::DataHeader)) {
      XTRACE(DATA, DEB, "Not enough data left for header: %u", BytesLeft);
      Stats.ErrorHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    auto DataHdrPtr = (ReadoutParser::DataHeader *)DataPtr;

    if (BytesLeft < DataHdrPtr->DataLength) {
      XTRACE(DATA, DEB, "Data size mismatch, header says %u got %d",
             DataHdrPtr->DataLength, BytesLeft);
      Stats.ErrorHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    if (DataHdrPtr->RingId > MaxRingId or DataHdrPtr->FENId > MaxFENId) {
      XTRACE(DATA, WAR, "Invalid RingId (%u) or FENId (%u)", DataHdrPtr->RingId,
             DataHdrPtr->FENId);
      Stats.ErrorHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    XTRACE(DATA, DEB, "Ring %u, FEN %u, Length %u", DataHdrPtr->RingId,
           DataHdrPtr->FENId, DataHdrPtr->DataLength);
    Stats.Headers++;

    if (DataHdrPtr->DataLength < sizeof(DataParser::DreamReadout)) {
      XTRACE(DATA, WAR, "Invalid data length %u", DataHdrPtr->DataLength);
      Stats.ErrorHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    ParsedData CurrentDataSection;
    CurrentDataSection.RingId = DataHdrPtr->RingId;
    CurrentDataSection.FENId = DataHdrPtr->FENId;

    // Loop through data here
    auto ReadoutsInDataSection =
        (DataHdrPtr->DataLength - DataHeaderSize) / DreamReadoutSize;
    for (unsigned int i = 0; i < ReadoutsInDataSection; i++) {
      auto Data = (DreamReadout *)((char *)DataHdrPtr + DataHeaderSize +
                                   i * DreamReadoutSize);
      // XTRACE(DATA, DEB, "%3u: ring %u, fen %u, t(%11u,%11u) SeqNo %6u TubeId
      // %3u , A 0x%04x B "
      //                   "0x%04x C 0x%04x D 0x%04x",
      //        i, DataHdrPtr->RingId, DataHdrPtr->FENId,
      //        Data->TimeHigh, Data->TimeLow, Data->DataSeqNum, Data->TubeId,
      //        Data->AmpA, Data->AmpB, Data->AmpC, Data->AmpD);

      CurrentDataSection.Data.push_back(*Data);
      ParsedReadouts++;
      Stats.Readouts++;
    }
    Result.push_back(CurrentDataSection);
    BytesLeft -= DataHdrPtr->DataLength;
    DataPtr += DataHdrPtr->DataLength;
  }

  return ParsedReadouts;
}
} // namespace Dream
