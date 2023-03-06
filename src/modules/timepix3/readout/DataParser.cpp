// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#include <timepix3/readout/DataParser.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Timepix3 {

// Assume we start after the PacketHeader
int DataParser::parse(const char *Buffer, unsigned int Size) {
  Result.clear();
  unsigned int ParsedReadouts = 0;

  unsigned int BytesLeft = Size;
  char *DataPtr = (char *)Buffer;

  while (BytesLeft) {
    uint64_t dataBytes;

    if (BytesLeft <= sizeof(dataBytes)){
      //TODO add some error handling here
    }
    // Copy 2 bytes starting from offset 0 into dcolBytes variable
    memcpy(&dataBytes, DataPtr, sizeof(dataBytes));

    Timepix3Readout Data;
    Data.dcol = (dataBytes & 0x0FE0000000000000) >> 52;
    Data.spix = (dataBytes & 0x001F800000000000) >> 45;
    Data.pix = (dataBytes & 0x0000700000000000) >> 44;
    uint64_t timeData = (dataBytes & 0x00000FFFFFFF0000) >> 16;
    Data.spidr_time = dataBytes & 0x000000000000FFFF;
    Data.ToA = (timeData & 0x0FFFC000) >> 14;
    Data.FToA = timeData & 0xF;
    Data.ToT = ((timeData & 0x00003FF0) >> 4) * 25;
   

    XTRACE(DATA, DEB, "Processed readout, dcol = %u, spix = %u, pix = %u, spidr_time = %u, ToA = %u, FToA = %u, ToT = %u", Data.dcol, Data.spix, Data.pix, Data.spidr_time, Data.ToA, Data.FToA, Data.ToT);
    ParsedReadouts++;
    Stats.Readouts++;

    Result.push_back(Data);
    BytesLeft -= sizeof(dataBytes);
    DataPtr += sizeof(dataBytes);
  }

  return ParsedReadouts;
}
} // namespace Timepix3
