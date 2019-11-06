/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */

#include <common/Trace.h>
#include <loki/readout/DataParser.h>
#include <readout/Readout.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Loki {

constexpr unsigned int DataHeaderSize{sizeof(Readout::DataHeader)};
constexpr unsigned int LokiReadoutSize{sizeof(DataParser::LokiReadout)};

// Assume we start after the PacketHeader
int DataParser::parse(const char *Buffer, unsigned int Size) {
  Result.clear();
  unsigned int ParsedReadouts = 0;

  unsigned int BytesLeft = Size;
  char *DataPtr = (char *)Buffer;

  while (BytesLeft) {
    // Parse Data Header
    if (BytesLeft < sizeof(Readout::DataHeader)) {
      XTRACE(DATA, DEB, "Not enough data left for header");
      Stats.ErrorHeaders++;
      Stats.ErrorBytes += BytesLeft;
      return ParsedReadouts;
    }

    auto DataHdrPtr = (Readout::DataHeader *)DataPtr;

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
    Stats.Headers++;

    ParsedData CurrentDataSection;
    CurrentDataSection.RingId = DataHdrPtr->RingId;
    CurrentDataSection.FENId = DataHdrPtr->FENId;

    // Loop through data here
    auto ReadoutsInDataSection = (DataHdrPtr->DataLength - DataHeaderSize) / LokiReadoutSize;
    for (unsigned int i = 0; i < ReadoutsInDataSection; i++) {
      auto Data = (LokiReadout *)((char *)DataHdrPtr + DataHeaderSize +
                                  i * LokiReadoutSize);
      XTRACE(DATA, DEB, "%u: ring %u, fen %u, t(%u,%u) FpgaTube 0x%02x, A 0x%04x B "
                        "0x%04x C 0x%04x D 0x%04x",
             i, DataHdrPtr->RingId, DataHdrPtr->FENId,
             Data->TimeHigh, Data->TimeLow, Data->FpgaAndTube, Data->AmpA,
             Data->AmpB, Data->AmpC, Data->AmpD);

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
}
