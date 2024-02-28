// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of TTL monitor
///
/// Stat counters accumulate
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <cbm/geometry/Parser.h>

namespace TTLMonitor {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

// Assume we start after the Common PacketHeader
void Parser::parse(ESSReadout::Parser::PacketDataV0 &PacketData) {
  Result.clear();

  char *Buffer = (char *)PacketData.DataPtr;
  unsigned int Size = PacketData.DataLength;
  ESSReadout::ESSTime &TimeRef = PacketData.Time;

  if (Buffer == nullptr) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data pointer");
    return;
  }

  if (Size == 0) {
    Stats.NoData++;
    XTRACE(DATA, WAR, "Data size is 0");
    return;
  }

  if (Size % DataLength != 0) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Bad data length - %d should be multiple of %d", Size,
           DataLength);
    return;
  }

  Parser::Data *DataPtr = (struct Data *)Buffer;
  for (unsigned int i = 0; i < Size / DataLength; i++) {
    Stats.Readouts++;
    Parser::Data Readout = DataPtr[i];
    if (Readout.FiberId > MaxFiberId) {
      XTRACE(DATA, WAR, "Invalid FiberId %d (Max is %d)", Readout.FiberId,
             MaxFiberId);
      Stats.ErrorFiber++;
      continue;
    }

    if (Readout.FENId > MaxFENId) {
      XTRACE(DATA, WAR, "Invalid FENId %d (valid: 0 - %d)", Readout.FENId,
             MaxFENId);
      Stats.ErrorFEN++;
      continue;
    }

    if (Readout.DataLength != DataLength) {
      XTRACE(DATA, WAR, "Invalid header length %d - must be %d bytes",
             Readout.DataLength, DataLength);
      Stats.ErrorDataLength++;
      continue;
    }

    if (Readout.ADC == 0) {
      XTRACE(DATA, WAR, "Invalid ADC (0)");
      Stats.ErrorADC++;
    }

    if (Readout.TimeLow > ESSReadout::MaxFracTimeCount) {
      XTRACE(DATA, WAR, "Invalid TimeLO %u (max is %u)", Readout.TimeLow,
             ESSReadout::MaxFracTimeCount);
      Stats.ErrorTimeFrac++;
      continue;
    }

    // Check for negative TOFs
    auto TimeOfFlight = TimeRef.getTOF(Readout.TimeHigh, Readout.TimeLow);
    XTRACE(DATA, DEB, "PulseTime     %" PRIu64 ", TimeStamp %" PRIu64 " ",
           TimeRef.TimeInNS, TimeOfFlight);

    if (TimeOfFlight == TimeRef.InvalidTOF) {
      XTRACE(DATA, WAR, "No valid TOF from PulseTime or PrevPulseTime");
      // Counters are incremented in ESSTime.h
      continue;
    }

    Result.push_back(Readout);
  }

  return;
}

} // namespace TTLMonitor
