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

// Assume we start after the Common PacketHeader
int VMM3Parser::parse(const char *Buffer, unsigned int Size) {
  Result.clear();
  unsigned int ParsedReadouts = 0;

  if (Buffer == nullptr) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data pointer");
    return ParsedReadouts;
  }

  if (Size % 20 != 0) {
    Stats.ErrorSize++;
    XTRACE(DATA, WAR, "Invalid data length - %d should be multiple of 20", Size);
    return ParsedReadouts;
  }

  nonstd::span<const VMM3Data> Data((struct VMM3Data *)Buffer, Size);

  for (auto & Readout : Data) {
    if (Readout.RingId > MaxRingId) {
      Stats.ErrorRing++;
      continue;
    }

    if (Readout.FENId > MaxFENId) {
      Stats.ErrorFEN++;
      continue;
    }

    if ((Readout.GEO & 0x80) == 0) {
      Stats.DataReadout++;
    } else {
      Stats.CalibReadout++;
    }

    Result.push_back(Readout);
    Stats.Readouts++;
  }

  return ParsedReadouts;
}
