// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of TTL monitor
///
/// Stat counters accumulate
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <ttlmonitor/geometry/Parser.h>

#include <common/memory/span.hpp>

namespace TTLMonitor {

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

// Assume we start after the Common PacketHeader
int Parser::parse(ESSReadout::Parser::PacketDataV0 &PacketData) {
  Result.clear();
  uint32_t GoodReadouts{0};

  char *Buffer = (char *)PacketData.DataPtr;
  unsigned int Size = PacketData.DataLength;
  ESSReadout::ESSTime &TimeRef = PacketData.Time;

  if (Buffer == nullptr) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data pointer");
    return GoodReadouts;
  }

  if (Size % DataLength != 0) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Bad data length - %d should be multiple of %d", Size, DataLength);
    return GoodReadouts;
  }

  Parser::Data *DataPtr = (struct Data *)Buffer;
  for (unsigned int i = 0; i < Size / DataLength; i++) {
    Stats.Readouts++;
    Parser::Data Readout = DataPtr[i];
    if (Readout.RingId > MaxRingId) {
      XTRACE(DATA, WAR, "Invalid RingId %d (Max is %d)", Readout.RingId,
             MaxRingId);
      Stats.ErrorRing++;
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

    // Validation done, increment stats for decoded parameters
    GoodReadouts++;
    Result.push_back(Readout);
  }

  return GoodReadouts;
}

// void VMM3Parser::dumpReadoutToFile(
//     const VMM3Data &Data, const ESSReadout::Parser ESSReadoutParser,
//     std::shared_ptr<VMM3::ReadoutFile> DumpFile) {
//   VMM3::Readout CurrentReadout;
//   CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
//   CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
//   CurrentReadout.PrevPulseTimeHigh =
//       ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
//   CurrentReadout.PrevPulseTimeLow =
//       ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
//   CurrentReadout.EventTimeHigh = Data.TimeHigh;
//   CurrentReadout.EventTimeLow = Data.TimeLow;
//   CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
//   CurrentReadout.BC = Data.BC;
//   CurrentReadout.OTADC = Data.OTADC;
//   CurrentReadout.GEO = Data.GEO;
//   CurrentReadout.TDC = Data.TDC;
//   CurrentReadout.VMM = Data.VMM;
//   CurrentReadout.Channel = Data.Channel;
//   CurrentReadout.RingId = Data.RingId;
//   CurrentReadout.FENId = Data.FENId;
//
//   DumpFile->push(CurrentReadout);
// }
} // namespace ESSReadout
