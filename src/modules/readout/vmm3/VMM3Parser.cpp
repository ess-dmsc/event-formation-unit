// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of VMM3a data
//===----------------------------------------------------------------------===//

#include <readout/vmm3/VMM3Parser.h>

// Assume we start after the PacketHeader
int VMM3Parser::parse(const char __attribute__((unused)) *Buffer, unsigned int __attribute__((unused)) Size) {
  Result.clear();
  unsigned int ParsedReadouts = 0;

  // if () {}
  //
  // unsigned int BytesLeft = Size;
  // char *DataPtr = (char *)Buffer;
  //
  // while (BytesLeft) {
  //   // Parse Data Header
  //   if (BytesLeft < sizeof(ReadoutParser::DataHeader)) {
  //     XTRACE(DATA, WAR, "Not enough data left for header: %u", BytesLeft);
  //     Stats.ErrorDataHeaders++;
  //     Stats.ErrorBytes += BytesLeft;
  //     return ParsedReadouts;
  //   }
  //
  //   auto DataHdrPtr = (ReadoutParser::DataHeader *)DataPtr;
  //
  //   ///\todo clarify distinction between logical and physical rings
  //   // for now just divide by two
  //   DataHdrPtr->RingId = DataHdrPtr->RingId/2;
  //
  //   if (BytesLeft < DataHdrPtr->DataLength) {
  //     XTRACE(DATA, WAR, "Data size mismatch, header says %u got %d",
  //            DataHdrPtr->DataLength, BytesLeft);
  //     Stats.ErrorDataHeaders++;
  //     Stats.ErrorBytes += BytesLeft;
  //     return ParsedReadouts;
  //   }
  //
  //   ///\todo remove ad hoc conters sometime
  //   HeaderCounters[DataHdrPtr->RingId & 0xf][DataHdrPtr->FENId & 0xf]++;
  //
  //   if (DataHdrPtr->RingId > MaxRingId or DataHdrPtr->FENId > MaxFENId) {
  //     XTRACE(DATA, WAR, "Invalid RingId (%u) or FENId (%u)", DataHdrPtr->RingId,
  //            DataHdrPtr->FENId);
  //     Stats.ErrorDataHeaders++;
  //     Stats.ErrorBytes += BytesLeft;
  //     return ParsedReadouts;
  //   }
  //
  //   XTRACE(DATA, DEB, "Ring %u, FEN %u, Length %u", DataHdrPtr->RingId,
  //          DataHdrPtr->FENId, DataHdrPtr->DataLength);
  //   Stats.DataHeaders++;
  //
  //   if (DataHdrPtr->DataLength < sizeof(DataParser::LokiReadout)) {
  //     XTRACE(DATA, WAR, "Invalid data length %u", DataHdrPtr->DataLength);
  //     Stats.ErrorDataHeaders++;
  //     Stats.ErrorBytes += BytesLeft;
  //     return ParsedReadouts;
  //   }
  //
  //   ParsedData CurrentDataSection;
  //   CurrentDataSection.RingId = DataHdrPtr->RingId;
  //   CurrentDataSection.FENId = DataHdrPtr->FENId;
  //
  //   // Loop through data here
  //   auto ReadoutsInDataSection =
  //       (DataHdrPtr->DataLength - DataHeaderSize) / LokiReadoutSize;
  //   for (unsigned int i = 0; i < ReadoutsInDataSection; i++) {
  //     auto Data = (LokiReadout *)((char *)DataHdrPtr + DataHeaderSize +
  //                                 i * LokiReadoutSize);
  //     XTRACE(DATA, DEB,
  //            "%3u: ring %u, fen %u, t(%11u,%11u) SeqNo %6u TubeId %3u , A "
  //            "0x%04x B "
  //            "0x%04x C 0x%04x D 0x%04x",
  //            i, DataHdrPtr->RingId, DataHdrPtr->FENId, Data->TimeHigh,
  //            Data->TimeLow, Data->DataSeqNum, Data->TubeId, Data->AmpA,
  //            Data->AmpB, Data->AmpC, Data->AmpD);
  //
  //     CurrentDataSection.Data.push_back(*Data);
  //     ParsedReadouts++;
  //     Stats.Readouts++;
  //   }
  //   Result.push_back(CurrentDataSection);
  //   BytesLeft -= DataHdrPtr->DataLength;
  //   DataPtr += DataHdrPtr->DataLength;
  // }

  return ParsedReadouts;
}
