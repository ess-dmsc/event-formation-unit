// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of CBM devices
///
/// Stat counters accumulate
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/readout/Parser.h>

namespace cbm {

using namespace esstime;

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

// Assume we start after the Common PacketHeader
void Parser::parse(ESSReadout::Parser::PacketDataV0 &PacketData) {
  Result.clear();

  char *Buffer = (char *)PacketData.DataPtr;
  unsigned int Size = PacketData.DataLength;

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

  Parser::CbmReadout *Readout = (struct CbmReadout *)Buffer;
  for (unsigned int i = 0; i < Size / DataLength; i++) {

    Stats.Readouts++;
    if (Readout->FiberId > MaxFiberId) {
      XTRACE(DATA, WAR, "Invalid FiberId %d (Max is %d)", Readout->FiberId,
             MaxFiberId);
      Stats.ErrorFiber++;
      continue;
    }

    if (Readout->FENId > MaxFENId) {
      XTRACE(DATA, WAR, "Invalid FENId %d (valid: 0 - %d)", Readout->FENId,
             MaxFENId);
      Stats.ErrorFEN++;
      continue;
    }

    if (Readout->DataLength != DataLength) {
      XTRACE(DATA, WAR, "Invalid header length %d - must be %d bytes",
             Readout->DataLength, DataLength);
      Stats.ErrorDataLength++;
      continue;
    }

    if (Readout->TimeLow > ESSReadout::MaxFracTimeCount) {
      XTRACE(DATA, WAR, "Invalid TimeLO %u (max is %u)", Readout->TimeLow,
             ESSReadout::MaxFracTimeCount);
      Stats.ErrorTimeFrac++;
      continue;
    }

    if (Readout->Type > CbmType::MAX || Readout->Type < CbmType::MIN) {
      XTRACE(DATA, WAR, "Invalid data type %d (valid: %d - %d)", Readout->Type,
             CbmType::MIN, CbmType::MAX);
      Stats.ErrorType++;
      continue;
    }

    /// Check readouts are valid according to the supported data types
    if (!isValidType(Readout->Type)) {
      XTRACE(DATA, WAR, "Unsupported type %d", Readout->Type);
      Stats.ErrorType++;
      continue;
    }

    Result.push_back(*Readout);

    Readout++;
  }

  return;
}

bool Parser::isValidType(uint8_t Type) const {
  switch (Type) {
  case CbmType::EVENT_0D:
  case CbmType::EVENT_2D:
  case CbmType::IBM:
    return true;
  default:
    return false;
  }
}

} // namespace cbm
