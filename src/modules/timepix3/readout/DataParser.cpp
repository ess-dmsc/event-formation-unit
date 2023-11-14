// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#include "common/dataflow/DataObserverTemplate.h"
#include "readout/DataEventTypes.h"
#include "readout/TimingEventHandler.h"
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <iostream>
#include <timepix3/readout/DataParser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

DataParser::DataParser(struct Counters &counters,
                       TimingEventHandler &timingEventHandler)
    : Stats(counters), TimingSyncHandler(timingEventHandler),
      TdcDataObservable() {
  PixelResult.reserve(MaxReadoutsInPacket);
  TdcDataObservable.subscribe(&TimingSyncHandler);
};

int DataParser::parse(const char *Buffer, unsigned int Size) {
  XTRACE(DATA, DEB, "parsing data, size is %u", Size);
  PixelResult.clear();
  unsigned int ParsedReadouts = 0;

  unsigned int BytesLeft = Size;
  char *DataPtr = (char *)Buffer;

  // packets in timepix3 datastream are either from the camera or the EVR system
  // if from the EVR system, they will be 24 bits, and will contain pulse time
  // information. If they are 24 bits but not type = 1, then it is a camera
  // packet
  if (Size == sizeof(EVRReadout)) {
    XTRACE(DATA, DEB, "size is 24, could be EVR timestamp");
    EVRReadout *Data = (EVRReadout *)((char *)DataPtr);
    if (Data->Type == EVR_READOUT_TYPE) {
      XTRACE(DATA, DEB,
             "Processed readout, packet type = %u, counter = %u, pulsetime "
             "seconds = %u, "
             "pulsetime nanoseconds = %u, previous pulsetime seconds = %u, "
             "previous pulsetime nanoseconds = %u",
             1, Data->Counter, Data->PulseTimeSeconds,
             Data->PulseTimeNanoSeconds, Data->PrevPulseTimeSeconds,
             Data->PrevPulseTimeNanoSeconds);
      Stats.EVRTimestampReadouts++;
      LastEVRTime =
          Data->PulseTimeSeconds * 1000000000 + Data->PulseTimeNanoSeconds;
      return 1;
    }
    XTRACE(DATA, DEB,
           "Not type = 1, not an EVR timestamp, processing as normal");
  }

  while (BytesLeft) {
    uint64_t DataBytes;

    if (BytesLeft < sizeof(DataBytes)) {
      // TODO add some error handling here
      // Maybe add a counter about demaged chunks
      XTRACE(DATA, DEB, "not enough bytes left, %u", BytesLeft);
      break;
    }
    // Copy 8 bytes into DataBytes variable
    memcpy(&DataBytes, DataPtr, sizeof(DataBytes));

    // regardless of readout type, the type variable is always in the same place
    // we read it here
    uint8_t ReadoutType = (DataBytes & TYPE_MASK) >> TYPE_OFFS;

    // pixel readout, identifies where a pixel on the camera was activated
    if (ReadoutType == 11) {
      Timepix3PixelReadout Data;

      // mask and offset values are defined in DataParser.h
      Data.Dcol = (DataBytes & PIXEL_DCOL_MASK) >> PIXEL_DCOL_OFFSET;
      Data.Spix = (DataBytes & PIXEL_SPIX_MASK) >> PIXEL_SPIX_OFFSET;
      Data.Pix = (DataBytes & PIXEL_PIX_MASK) >> PIXEL_PIX_OFFSET;
      Data.ToA = (DataBytes & PIXEL_TOA_MASK) >> PIXEL_TOA_OFFSET;

      // TOT has a unit of 25ns and we convert according to that.
      Data.ToT = ((DataBytes & PIXEL_TOT_MASK) >> PIXEL_TOT_OFFSET) * 25;
      Data.FToA = (DataBytes & PIXEL_FTOA_MASK) >> PIXEL_FTOA_OFFSET;
      Data.SpidrTime = DataBytes & PIXEL_SPTIME_MASK;

      XTRACE(DATA, DEB,
             "Processed readout, ReadoutType = %u, Dcol = %u, Spix = %u, Pix = "
             "%u, SpidrTime = %u, ToA = %u, FToA = %u, ToT = %u",
             ReadoutType, Data.Dcol, Data.Spix, Data.Pix, Data.SpidrTime,
             Data.ToA, Data.FToA, Data.ToT);

      // toa formula is based on information from the timepix3 manual provided
      // with the camera
      uint64_t toa = uint64_t(409600 * uint64_t(Data.SpidrTime) +
                              25 * uint64_t(Data.ToA) - 1.5625 * Data.FToA);
      XTRACE(DATA, DEB, "ToA in nanoseconds: %u", toa);
      ParsedReadouts++;
      Stats.PixelReadouts++;

      // LastTDCTime is the latest seen time from a TDC readout, and this
      // counter is used to see how strictly in order the pixel readouts are in
      // relation to this. Pixel readouts from before the last TDC time belong
      // to the previous pulse, and may need to be treated differently

      if (toa < TimingSyncHandler.getLastTDCTimestamp()) {
        XTRACE(DATA, DEB, "Pixel readout from before TDC");
        Stats.PixelReadoutFromBeforeTDC++;
      }

      PixelResult.push_back(Data);

      // TDC readout type, indicating when the camera received a TDC pulse. In
      // the ESS setup, this should correspond to an EVR pulse, indicating the
      // start of a new pulse.
    } else if (ReadoutType == 6) {

      // mask and offset values are defined in DataParser.h
      TDCDataEvent *Data = new TDCDataEvent(
          (DataBytes & TDC_TYPE_MASK) >> TDC_TYPE_OFFSET,
          (DataBytes & TDC_TRIGGERCOUNTER_MASK) >> TDC_TRIGGERCOUNTER_OFFSET,
          (DataBytes & TDC_TIMESTAMP_MASK) >> TDC_TIMESTAMP_OFFSET,
          (DataBytes & TDC_STAMP_MASK) >> TDC_STAMP_OFFSET);

      ParsedReadouts++;
      Stats.TDCReadouts++;

      // TDC readouts can belong to one of two channels, and can either indicate
      // the rising or the falling edge of the signal. The camera setup will
      // determine which of these are sent.
      /// \todo: Review that it's necessary monitor which type of TDC we
      /// received. Probably this is not important.
      if (Data->Type == 15) {
        Stats.TDC1RisingReadouts++;
        TdcDataObservable.publishData(*Data);
      } else if (Data->Type == 10) {
        Stats.TDC1FallingReadouts++;
        TdcDataObservable.publishData(*Data);
      } else if (Data->Type == 14) {
        Stats.TDC2RisingReadouts++;
        TdcDataObservable.publishData(*Data);
      } else if (Data->Type == 11) {
        Stats.TDC2FallingReadouts++;
        TdcDataObservable.publishData(*Data);
      } else {
        // this should never happen - if it does something has gone wrong with
        // the data format or parsing
        Stats.UnknownTDCReadouts++;
      }

      // global timestamps are a readout allowing for longer periods of time to
      // be recorded before "overflowing" to 0. This readout type isn't expected
      // to be used in production, currently.
    } else if (ReadoutType == 4) {
      Timepix3GlobalTimeReadout Data;

      // mask and offset values are defined in DataParser.h
      Data.Timestamp =
          (DataBytes & GLOBAL_TIMESTAMP_MASK) >> GLOBAL_TIMESTAMP_OFFSET;
      Data.Stamp = (DataBytes & GLOBAL_STAMP_MASK) >> GLOBAL_STAMP_OFFSET;

      XTRACE(DATA, DEB,
             "Processed readout, ReadoutType = %u, Timestamp = %u, Stamp = %u",
             ReadoutType, Data.Timestamp, Data.Stamp);
      ParsedReadouts++;
      Stats.GlobalTimestampReadouts++;
    } else {
      // we sometimes see packet type 7 here, which accompanies a lot of control
      // signals
      XTRACE(DATA, WAR, "Unknown packet type: %u", ReadoutType);
      Stats.UndefinedReadouts++;
    }
    BytesLeft -= sizeof(DataBytes);
    DataPtr += sizeof(DataBytes);
  }

  return ParsedReadouts;
}

} // namespace Timepix3
