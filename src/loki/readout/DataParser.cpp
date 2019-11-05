/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */

#include <common/Trace.h>
#include <loki/readout/DataParser.h>
#include <readout/Readout.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {

constexpr unsigned int DataHeaderSize{sizeof(Readout::DataHeader)};
constexpr unsigned int LokiReadoutSize{sizeof(DataParser::LokiReadout)};

// Assume we start after the PacketHeader
int DataParser::parse(const char *Buffer, unsigned int Size) {
  unsigned int BytesLeft = Size;
  char *DataPtr = (char *)Buffer;

  // Add while ()

  // Parse Data Header
  if (BytesLeft < sizeof(Readout::DataHeader)) {
    XTRACE(DATA, DEB, "Not enough data left for header");
    Stats.ErrorBytes += BytesLeft;
    return Stats.Readouts;
  }

  auto DataHdrPtr = (Readout::DataHeader *)DataPtr;

  if (BytesLeft < DataHdrPtr->DataLength) {
    XTRACE(DATA, DEB, "Data size mismatch, header says %u got %d",
           DataHdrPtr->DataLength, BytesLeft);
    Stats.ErrorBytes += BytesLeft;
    return Stats.Readouts;
  }

  if (DataHdrPtr->RingId > MaxRingId or DataHdrPtr->FENId > MaxFENId) {
    XTRACE(DATA, WAR, "Invalid RingId (%u) or FENId (%u)", DataHdrPtr->RingId,
           DataHdrPtr->FENId);
    Stats.ErrorHeaders++;
    Stats.ErrorBytes += BytesLeft;
    return Stats.Readouts;
  }

  // Loop through data here
  auto NbReadouts = (DataHdrPtr->DataLength - DataHeaderSize) / LokiReadoutSize;
  XTRACE(DATA, DEB, "Data Section has %d readouts", NbReadouts);
  for (unsigned int i = 0; i < NbReadouts; i++) {
    auto Data = (LokiReadout *)((char *)DataHdrPtr + DataHeaderSize +
                                i * LokiReadoutSize);
    XTRACE(DATA, DEB, "%u: t(%u,%u) FpgaTube 0x%02x, A 0x%04x B "
                      "0x%04x C 0x%04x D 0x%04x",
           i, Data->TimeHigh, Data->TimeLow, Data->FpgaAndTube, Data->AmpA,
           Data->AmpB, Data->AmpC, Data->AmpD);
    Stats.Readouts++;
  }

  BytesLeft -= DataHdrPtr->DataLength;
  DataPtr += DataHdrPtr->DataLength;

  // add loop below

  return Stats.Readouts;
}
}
