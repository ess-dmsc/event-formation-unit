// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <timepix3/readout/DataParser.h>
#include <iostream>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

// Assume we start after the PacketHeader
int DataParser::parse(const char *Buffer, unsigned int Size) {
  XTRACE(DATA, DEB, "parsing data, size is %u", Size);
  PixelResult.clear();
  unsigned int ParsedReadouts = 0;

  unsigned int BytesLeft = Size;
  char *DataPtr = (char *)Buffer;

  if(Size == 24){
    XTRACE(DATA, DEB, "size is 24, could be EVR timestamp");
    EVRTimeReadout *Data = (EVRTimeReadout *)((char *)DataPtr);
    if (Data->Type == 1){
      XTRACE(DATA, DEB,
          "Processed readout, packet type = %u, counter = %u, pulsetime seconds = %u, "
          "pulsetime nanoseconds = %u, previous pulsetime seconds = %u, "
          "previous pulsetime nanoseconds = %u",
          1, Data->Counter, Data->PulseTimeSeconds, Data->PulseTimeNanoSeconds,
          Data->PrevPulseTimeSeconds, Data->PrevPulseTimeNanoSeconds);
        Stats.EVRTimestampReadouts++;
        LastEVRTime = Data->PulseTimeSeconds * 1000000000 + Data->PulseTimeNanoSeconds;
      return 1;
    }
    XTRACE(DATA, DEB, "Not type = 1, not an EVR timestamp, processing as normal");
  }

  while (BytesLeft) {
    uint64_t DataBytes;

    if (BytesLeft < sizeof(DataBytes)) {
      // TODO add some error handling here
      XTRACE(DATA, DEB, "not enough bytes left, %u", BytesLeft);
      break;
    }
    // Copy 8 bytes into DataBytes variable
    memcpy(&DataBytes, DataPtr, sizeof(DataBytes));

    uint8_t PacketType = (DataBytes & TYPE_MASK) >> TYPE_OFFS;
    if (PacketType == 11) {
      Timepix3PixelReadout Data;

      Data.Dcol = (DataBytes & PIXEL_DCOL_MASK) >> PIXEL_DCOL_OFFS;
      Data.Spix = (DataBytes & PIXEL_SPIX_MASK) >> PIXEL_SPIX_OFFS;
      Data.Pix = (DataBytes & PIXEL_PIX_MASK) >> PIXEL_PIX_OFFS;
      Data.ToA = (DataBytes & PIXEL_TOA_MASK) >> PIXEL_TOA_OFFS;
      Data.ToT = ((DataBytes & PIXEL_TOT_MASK) >> PIXEL_TOT_OFFS) * 25;
      Data.FToA = (DataBytes & PIXEL_FTOA_MASK) >> PIXEL_FTOA_OFFS;
      Data.SpidrTime = DataBytes & PIXEL_SPTIME_MASK;

      XTRACE(DATA, DEB,
             "Processed readout, PacketType = %u, Dcol = %u, Spix = %u, Pix = "
             "%u, SpidrTime = %u, ToA = %u, FToA = %u, ToT = %u",
             PacketType, Data.Dcol, Data.Spix, Data.Pix, Data.SpidrTime,
             Data.ToA, Data.FToA, Data.ToT);
      XTRACE(DATA, DEB, "ToA in nanoseconds: %u, previous TDC timestamp in nanoseconds: %u, difference: %u", int(409600 * Data.SpidrTime + 25 * Data.ToA + 1.5625 * Data.FToA), LastTDCTime, int(409600 * Data.SpidrTime + 25 * Data.ToA + 1.5625 * Data.FToA) - LastTDCTime);
      ParsedReadouts++;
      Stats.PixelReadouts++;

      PixelResult.push_back(Data);
    } else if (PacketType == 6) {
      Timepix3TDCReadout Data;
      Data.Type = (DataBytes & 0x0F00000000000000) >> 56;
      Data.TriggerCounter = (DataBytes & 0x00FFF00000000000) >> 44;
      Data.Timestamp = (DataBytes & 0x00000FFFFFFFFE00) >> 9;
      Data.Stamp = (DataBytes & 0x00000000000001E0) >> 5;

      XTRACE(DATA, DEB,
             "Processed readout, PacketType = %u, trigger_counter = %u, "
             "timestamp = %u, stamp = %u",
             PacketType, Data.TriggerCounter, Data.Timestamp, Data.Stamp);
      ParsedReadouts++;
      Stats.TDCReadouts++;
      LastTDCTime = 3.125 * Data.Timestamp + 0.26 * Data.Stamp;

      if (Data.Type == 15) {
        Stats.TDC1RisingReadouts++;
      } else if (Data.Type == 10) {
        Stats.TDC1FallingReadouts++;
      } else if (Data.Type == 14) {
        Stats.TDC2RisingReadouts++;
      } else if (Data.Type == 11) {
        Stats.TDC2FallingReadouts++;
      }
      // else {
      //   Stats.UnknownTDC++;
      // }
    } else if (PacketType == 4) {
      Timepix3GlobalTimeReadout Data;

      Data.Timestamp = (DataBytes & 0x00FFFFFFFFFFFF00) >> 8;
      Data.Stamp = (DataBytes & 0x00000000000000F0) >> 4;

      XTRACE(DATA, DEB,
             "Processed readout, PacketType = %u, Timestamp = %u, Stamp = %u",
             PacketType, Data.Timestamp, Data.Stamp);
      ParsedReadouts++;
      Stats.GlobalTimestampReadouts++;

    } else {
      XTRACE(DATA, WAR, "Unknown packet type: %u", PacketType);
      Stats.UndefinedReadouts++;
    }
    BytesLeft -= sizeof(DataBytes);
    DataPtr += sizeof(DataBytes);
  }

  return ParsedReadouts;
}
} // namespace Timepix3
