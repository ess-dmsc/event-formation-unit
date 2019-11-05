/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */

#include <loki/readout/DataParser.h>
#include <common/Trace.h>
#include <readout/Readout.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {

// Assume we start after the PacketHeader
int DataParser::parse(const char * Buffer, unsigned int Size) {

  unsigned int BytesLeft = Size;

  if (BytesLeft < sizeof(Readout::DataHeader)) {
    Stats.ErrorBytes += BytesLeft;
    XTRACE(DATA, DEB, "Not enough data left for header");
    return Stats.Readouts;
  }


  auto DataHdrPtr = (Readout::DataHeader *)Buffer;

  if (BytesLeft < DataHdrPtr->DataLength) {
    XTRACE(DATA, DEB, "Data size mismatch, header says %u got %d",
       DataHdrPtr->DataLength, BytesLeft);
    Stats.ErrorBytes += BytesLeft;
    return Stats.Readouts;
  }

  if (DataHdrPtr->RingId > MaxRingId or DataHdrPtr->FENId > MaxFENId) {
    XTRACE(DATA, WAR, "Invalid RingId (%u) or FENId (%u)",
       DataHdrPtr->RingId, DataHdrPtr->FENId);
    Stats.ErrorHeaders++;
    Stats.ErrorBytes += BytesLeft;
    return Stats.Readouts;
  }

  return Stats.Readouts;
}

}
