// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of VMM3a data
//===----------------------------------------------------------------------===//

#include <common/span.hpp>
#include <common/Trace.h>
#include <readout/vmm3/VMM3Parser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

// Assume we start after the Common PacketHeader
int VMM3Parser::parse(const char *Buffer, unsigned int Size) {
  Result.clear();
  uint32_t GoodReadouts{0};

  if (Buffer == nullptr) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data pointer");
    return GoodReadouts;
  }

  if (Size % 20 != 0) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data length - %d should be multiple of 20", Size);
    return GoodReadouts;
  }

  VMM3Parser::VMM3Data * DataPtr = (struct VMM3Data *)Buffer;
  for (unsigned int i = 0; i < Size/20; i++) {
    Stats.Readouts++;
    VMM3Parser::VMM3Data Readout = DataPtr[i];
    if (Readout.RingId > MaxRingId) {
      XTRACE(DATA, WAR, "Invalid RingId %d (Max is %d)", Readout.RingId, MaxRingId);
      Stats.ErrorRing++;
      continue;
    }

    if ((Readout.FENId > MaxFENId) or (Readout.FENId == 0))  {
      XTRACE(DATA, WAR, "Invalid FENId %d (valid: 1 - %d)", Readout.FENId, MaxFENId);
      Stats.ErrorFEN++;
      continue;
    }

    if (Readout.DataLength != 20)  {
      XTRACE(DATA, WAR, "Invalid header length - must be 20 bytes", Readout.DataLength);
      Stats.ErrorDataLength++;
      continue;
    }

    if (Readout.TimeLow > MaxFracTimeCount)  {
      XTRACE(DATA, WAR, "Invalid TimeLO %u (max is %u)", Readout.TimeLow, MaxFracTimeCount);
      Stats.ErrorTimeFrac++;
      continue;
    }

    if (Readout.BC > MaxBCValue)  {
      XTRACE(DATA, WAR, "Invalid BC %u (max is %u)", Readout.BC, MaxBCValue);
      Stats.ErrorBC++;
      continue;
    }

    if ((Readout.OTADC & ADCMask) > MaxADCValue) {
      XTRACE(DATA, WAR, "Invalid TDC %u (max is %u)", Readout.OTADC & 0x7fff, MaxADCValue);
      Stats.ErrorADC++;
      continue;
    }


    // Validation done, increment stats for decoded parameters

    if (Readout.OTADC & OverThresholdMask) {
      Stats.OverThreshold++;
    }

    if ((Readout.GEO & 0x80) == 0) {
      Stats.DataReadout++;
    } else {
      Stats.CalibReadout++;
    }

    GoodReadouts++;
    Result.push_back(Readout);
  }

  return GoodReadouts;
}
