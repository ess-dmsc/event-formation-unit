// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of VMM3a data
///
/// Stat counters accumulate
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/readout/vmm3/VMM3Parser.h>

#include <common/memory/span.hpp>

namespace ESSReadout {

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

// Assume we start after the Common PacketHeader
int VMM3Parser::parse(Parser::PacketDataV0 &PacketData) {
  Result.clear();
  uint32_t GoodReadouts{0};

  char *Buffer = (char *)PacketData.DataPtr;
  unsigned int Size = PacketData.DataLength;
  ESSTime &TimeRef = PacketData.Time;

  if (Buffer == nullptr) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data pointer");
    return GoodReadouts;
  }

  if (Size % DataLength != 0) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data length - %d should be multiple of %d", Size,
           DataLength);
    return GoodReadouts;
  }

  VMM3Parser::VMM3Data *DataPtr = (struct VMM3Data *)Buffer;
  for (unsigned int i = 0; i < Size / DataLength; i++) {
    Stats.Readouts++;
    VMM3Parser::VMM3Data Readout = DataPtr[i];

    

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

    if (Readout.TimeLow > MaxFracTimeCount) {
      XTRACE(DATA, WAR, "Invalid TimeLO %u (max is %u)", Readout.TimeLow,
             MaxFracTimeCount);
      Stats.ErrorTimeFrac++;
      continue;
    }

    // Check for negative TOFs
    ///\todo Missing TDC correction
    auto TimeOfFlight = TimeRef.getTOF(Readout.TimeHigh, Readout.TimeLow);
    XTRACE(DATA, DEB, "PulseTime     %" PRIu64 ", TimeStamp %" PRIu64 " ",
           TimeRef.TimeInNS, TimeOfFlight);

    if (TimeOfFlight == TimeRef.InvalidTOF) {
      XTRACE(DATA, WAR, "No valid TOF from PulseTime or PrevPulseTime");
      continue;
    }

    if (Readout.BC > MaxBCValue) {
      XTRACE(DATA, WAR, "Invalid BC %u (max is %u)", Readout.BC, MaxBCValue);
      Stats.ErrorBC++;
      continue;
    }

    // If monitor there are no invalid ADC values
    if (not IsMonitor) {
      if ((Readout.OTADC & ADCMask) > MaxADCValue) {
        XTRACE(DATA, WAR, "Invalid ADC %u (max is %u)", Readout.OTADC & 0x7fff,
               MaxADCValue);
        Stats.ErrorADC++;
        continue;
      }
    }
    XTRACE(DATA, DEB, "Valid OTADC %u", Readout.OTADC);

    // So far no checks for GEO and TDC

    if (Readout.VMM > MaxVMMValue) {
      XTRACE(DATA, WAR, "Invalid VMM %u (max is %u)", Readout.VMM, MaxVMMValue);
      Stats.ErrorVMM++;
      continue;
    }

    if (Readout.Channel > MaxChannelValue) {
      XTRACE(DATA, WAR, "Invalid Channel %u (max is %u)", Readout.Channel,
             MaxChannelValue);
      Stats.ErrorChannel++;
      continue;
    }

    // Validation done, increment stats for decoded parameters

    if (Readout.OTADC & OverThresholdMask) {
      Stats.OverThreshold++;
    }

    if ((Readout.GEO & 0x80) == 0) {
      Stats.DataReadouts++;
    } else {
      Stats.CalibReadouts++;
    }

    GoodReadouts++;
    Result.push_back(Readout);
  }

  return GoodReadouts;
}

void VMM3Parser::dumpReadoutToFile(
      const VMM3Data &Data, const ESSReadout::Parser ESSReadoutParser, std::shared_ptr<VMM3::ReadoutFile> DumpFile) {
    VMM3::Readout CurrentReadout;
    CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
    CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
    CurrentReadout.PrevPulseTimeHigh =
        ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
    CurrentReadout.PrevPulseTimeLow =
        ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
    CurrentReadout.EventTimeHigh = Data.TimeHigh;
    CurrentReadout.EventTimeLow = Data.TimeLow;
    CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
    CurrentReadout.BC = Data.BC;
    CurrentReadout.OTADC = Data.OTADC;
    CurrentReadout.GEO = Data.GEO;
    CurrentReadout.TDC = Data.TDC;
    CurrentReadout.VMM = Data.VMM;
    CurrentReadout.Channel = Data.Channel;
    CurrentReadout.RingId = Data.RingId;
    CurrentReadout.FENId = Data.FENId;

    DumpFile->push(CurrentReadout);
}
} // namespace ESSReadout
