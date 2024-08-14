// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <timepix3/readout/DataParser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace timepixReadout;

DataParser::DataParser(struct Counters &counters) : Stats(counters) {}

int DataParser::parse(const char *Buffer, unsigned int Size) {
  XTRACE(DATA, DEB, "parsing data, size is %u", Size);

  unsigned int ParsedReadouts = 0;

  unsigned int BytesLeft = Size;
  char *DataPtr = (char *)Buffer;

  // packets in timepix3 datastream are either from the camera or the EVR system
  // if from the EVR system, they will be 24 bits, and will contain pulse time
  // information. If they are 24 bits but not type = 1, then it is a camera
  // packet

  if (Size == sizeof(struct EVRReadout)) {
    XTRACE(DATA, DEB, "size is 24, could be EVR timestamp");
    EVRReadout *Data = (EVRReadout *)((char *)DataPtr);

    if (Data->type == EVR_READOUT_TYPE) {
      XTRACE(DATA, DEB,
             "Processed readout, packet type = %u, counter = %u, pulsetime "
             "seconds = %u, "
             "pulsetime nanoseconds = %u, previous pulsetime seconds = %u, "
             "previous pulsetime nanoseconds = %u",
             1, Data->counter, Data->pulseTimeSeconds,
             Data->pulseTimeNanoSeconds, Data->prevPulseTimeSeconds,
             Data->prevPulseTimeNanoSeconds);

      DataEventObservable<EVRReadout>::publishData(*Data);
      Stats.EVRReadoutCounter++;
      return 1;
    }
    XTRACE(DATA, DEB,
           "Not type = 1, not an EVR timestamp, processing as normal");
  }

  while (BytesLeft) {
    uint64_t *DataBytesPtr;

    if (BytesLeft < sizeof(*DataBytesPtr)) {
      // TODO add some error handling here
      // Maybe add a counter about demaged chunks
      XTRACE(DATA, DEB, "not enough bytes left, %u", BytesLeft);
      break;
    }

    // we read the data as 64 bit integers, and then use bitmasks to extract the
    // relevant information. This is done to avoid alignment issues, as the data
    // is not guaranteed to be aligned to 64 bits
    DataBytesPtr = reinterpret_cast<uint64_t *>(DataPtr);

    // regardless of readout type, the type variable is always in the same place
    // we read it here
    uint8_t ReadoutType = (*DataBytesPtr & TYPE_MASK) >> TYPE_OFFS;

    // pixel readout, identifies where a pixel on the camera was activated
    if (ReadoutType == PIXEL_READOUT_TYPE_CONST) {

      PixelReadout pixelDataEvent(
          (*DataBytesPtr & PIXEL_DCOL_MASK) >> PIXEL_DCOL_OFFSET,
          (*DataBytesPtr & PIXEL_SPIX_MASK) >> PIXEL_SPIX_OFFSET,
          (*DataBytesPtr & PIXEL_PIX_MASK) >> PIXEL_PIX_OFFSET,
          (*DataBytesPtr & PIXEL_TOA_MASK) >> PIXEL_TOA_OFFSET,
          (*DataBytesPtr & PIXEL_TOT_MASK) >> PIXEL_TOT_OFFSET,
          (*DataBytesPtr & PIXEL_FTOA_MASK) >> PIXEL_FTOA_OFFSET,
          *DataBytesPtr & PIXEL_SPTIME_MASK);

      DataEventObservable<PixelReadout>::publishData(pixelDataEvent);

      ParsedReadouts++;
      Stats.PixelReadouts++;

      // TDC readout type, indicating when the camera received a TDC pulse. In
      // the ESS setup, this should correspond to an EVR pulse, indicating the
      // start of a new pulse.
    } else if (ReadoutType == TDC_READOUT_TYPE_CONST) {

      // mask and offset values are defined in DataParser.h
      TDCReadout tdcReadout(
          (*DataBytesPtr & TDC_TYPE_MASK) >> TDC_TYPE_OFFSET,
          (*DataBytesPtr & TDC_TRIGGERCOUNTER_MASK) >>
              TDC_TRIGGERCOUNTER_OFFSET,
          (*DataBytesPtr & TDC_TIMESTAMP_MASK) >> TDC_TIMESTAMP_OFFSET,
          (*DataBytesPtr & TDC_STAMP_MASK) >> TDC_STAMP_OFFSET);

      ParsedReadouts++;
      Stats.TDCReadoutCounter++;

      // TDC readouts can belong to one of two channels, and can either
      // indicate the rising or the falling edge of the signal. The camera
      // setup will determine which of these are sent.
      /// \todo: Review that it's necessary monitor which type of TDC we
      /// received. Probably this is not important.
      if (tdcReadout.type == TDC1_RISING_CONST) {
        Stats.TDC1RisingReadouts++;
        DataEventObservable<TDCReadout>::publishData(tdcReadout);
      } else if (tdcReadout.type == TDC1_FALLING_CONST) {
        Stats.TDC1FallingReadouts++;
        DataEventObservable<TDCReadout>::publishData(tdcReadout);
      } else if (tdcReadout.type == TDC2_RISING_CONST) {
        Stats.TDC2RisingReadouts++;
        DataEventObservable<TDCReadout>::publishData(tdcReadout);
      } else if (tdcReadout.type == TDC2_FALLING_CONST) {
        Stats.TDC2FallingReadouts++;
        DataEventObservable<TDCReadout>::publishData(tdcReadout);
      } else {
        // this should never happen - if it does something has gone wrong with
        // the data format or parsing
        Stats.UnknownTDCReadouts++;
      }

    } else {
      // we sometimes see packet type 7 here, which accompanies a lot of
      // control signals
      XTRACE(DATA, WAR, "Unknown packet type: %u", ReadoutType);
      Stats.UndefinedReadoutCounter++;
    }
    BytesLeft -= sizeof(*DataBytesPtr);
    DataPtr += sizeof(*DataBytesPtr);
  }
  return ParsedReadouts;
}

} // namespace Timepix3
