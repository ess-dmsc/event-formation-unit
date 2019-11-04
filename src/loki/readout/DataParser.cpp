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
    return Stats.Readouts;
  }
  auto DataHdrPtr = (Readout::DataHeader *)Buffer;
  if (DataHdrPtr->RingId > MaxRingId or DataHdrPtr->FENId > MaxFENId) {
    Stats.ErrorHeaders++;
    Stats.ErrorBytes += BytesLeft;
    return Stats.Readouts;
  }

  return Stats.Readouts;
}

}
