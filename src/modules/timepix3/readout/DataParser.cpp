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
    if (Data->type == 1){
      XTRACE(DATA, DEB,
          "Processed readout, packet type = %u, counter = %u, pulsetime seconds = %u, "
          "pulsetime nanoseconds = %u, previous pulsetime seconds = %u, "
          "previous pulsetime nanoseconds = %u",
          1, Data->counter, Data->pulseTimeSeconds, Data->pulseTimeNanoSeconds,
          Data->prevPulseTimeSeconds, Data->prevPulseTimeNanoSeconds);
        Stats.EVRTimestampReadouts++;
        lastEVRTime = Data->pulseTimeSeconds * 1000000000 + Data->pulseTimeNanoSeconds;
      return 1;
    }
    XTRACE(DATA, DEB, "Not type = 1, not an EVR timestamp, processing as normal");
  }

  while (BytesLeft) {
    uint64_t dataBytes;

    if (BytesLeft < sizeof(dataBytes)) {
      // TODO add some error handling here
      XTRACE(DATA, DEB, "not enough bytes left, %u", BytesLeft);
      break;
    }
    // Copy 8 bytes into dataBytes variable
    memcpy(&dataBytes, DataPtr, sizeof(dataBytes));

    uint8_t packet_type = (dataBytes & TYPE_MASK) >> TYPE_OFFS;
    if (packet_type == 11) {
      Timepix3PixelReadout Data;

      Data.dcol = (dataBytes & PIXEL_DCOL_MASK) >> PIXEL_DCOL_OFFS;
      Data.spix = (dataBytes & PIXEL_SPIX_MASK) >> PIXEL_SPIX_OFFS;
      Data.pix = (dataBytes & PIXEL_PIX_MASK) >> PIXEL_PIX_OFFS;
      Data.ToA = (dataBytes & PIXEL_TOA_MASK) >> PIXEL_TOA_OFFS;
      Data.ToT = ((dataBytes & PIXEL_TOT_MASK) >> PIXEL_TOT_OFFS) * 25;
      Data.FToA = (dataBytes & PIXEL_FTOA_MASK) >> PIXEL_FTOA_OFFS;
      Data.spidr_time = dataBytes & PIXEL_SPTIME_MASK;

      XTRACE(DATA, DEB,
             "Processed readout, packet_type = %u, dcol = %u, spix = %u, pix = "
             "%u, spidr_time = %u, ToA = %u, FToA = %u, ToT = %u",
             packet_type, Data.dcol, Data.spix, Data.pix, Data.spidr_time,
             Data.ToA, Data.FToA, Data.ToT);
      XTRACE(DATA, DEB, "ToA in nanoseconds: %u, previous TDC timestamp in nanoseconds: %u, difference: %u", int(409600 * Data.spidr_time + 25 * Data.ToA + 1.5625 * Data.FToA), lastTDCTime, int(409600 * Data.spidr_time + 25 * Data.ToA + 1.5625 * Data.FToA) - lastTDCTime);
      ParsedReadouts++;
      Stats.PixelReadouts++;

      PixelResult.push_back(Data);
    } else if (packet_type == 6) {
      Timepix3TDCReadout Data;
      Data.type = (dataBytes & 0x0F00000000000000) >> 56;
      Data.trigger_counter = (dataBytes & 0x00FFF00000000000) >> 44;
      Data.timestamp = (dataBytes & 0x00000FFFFFFFFE00) >> 9;
      Data.stamp = (dataBytes & 0x00000000000001E0) >> 5;

      XTRACE(DATA, DEB,
             "Processed readout, packet_type = %u, trigger_counter = %u, "
             "timestamp = %u, stamp = %u",
             packet_type, Data.trigger_counter, Data.timestamp, Data.stamp);
      ParsedReadouts++;
      Stats.TDCReadouts++;
      lastTDCTime = 3.125 * Data.timestamp + 0.26 * Data.stamp;

      if (Data.type == 15) {
        Stats.TDC1RisingReadouts++;
      } else if (Data.type == 10) {
        Stats.TDC1FallingReadouts++;
      } else if (Data.type == 14) {
        Stats.TDC2RisingReadouts++;
      } else if (Data.type == 11) {
        Stats.TDC2FallingReadouts++;
      }
      // else {
      //   Stats.UnknownTDC++;
      // }
    } else if (packet_type == 4) {
      Timepix3GlobalTimeReadout Data;

      Data.timestamp = (dataBytes & 0x00FFFFFFFFFFFF00) >> 8;
      Data.stamp = (dataBytes & 0x00000000000000F0) >> 4;

      XTRACE(DATA, DEB,
             "Processed readout, packet_type = %u, timestamp = %u, stamp = %u",
             packet_type, Data.timestamp, Data.stamp);
      ParsedReadouts++;
      Stats.GlobalTimestampReadouts++;

    } else {
      XTRACE(DATA, WAR, "Unknown packet type: %u", packet_type);
      Stats.UndefinedReadouts++;
    }
    BytesLeft -= sizeof(dataBytes);
    DataPtr += sizeof(dataBytes);
  }

  return ParsedReadouts;
}
} // namespace Timepix3
