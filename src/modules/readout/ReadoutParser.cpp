// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS readout data parser implementation
///
//===----------------------------------------------------------------------===//

#include <cstring>
#include <common/Trace.h>
#include <readout/ReadoutParser.h>
#include <arpa/inet.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#define VERSION_OFFSET 1

const unsigned int MaxUdpDataSize{8972};
const unsigned int MinDataSize{5}; // just pad, cookie and version

ReadoutParser::ReadoutParser() {
  std::memset(NextSeqNum, 0, sizeof(NextSeqNum));
}

int ReadoutParser::validate(const char *Buffer, uint32_t Size, uint8_t ExpectedType) {
  std::memset(&Packet, 0, sizeof(Packet));

  if (Buffer == nullptr or Size == 0) {
    XTRACE(PROCESS, WAR, "no buffer specified");
    Stats.ErrorBuffer++;
    return -ReadoutParser::EBUFFER;
  }

  if ((Size < MinDataSize) || (Size > MaxUdpDataSize)) {
    XTRACE(PROCESS, WAR, "Invalid data size (%u)", Size);
    Stats.ErrorSize++;
    return -ReadoutParser::ESIZE;
  }

  uint32_t Version = htons(*(uint16_t *)(Buffer));
  if ((Version >> 8) != 0) {
    XTRACE(PROCESS, WAR, "Padding is wrong (should be 0)");
    Stats.ErrorPad++;
    return -ReadoutParser::EHEADER;
  }

  if ((Version & 0xff) != 0x00) { //
    XTRACE(PROCESS, WAR, "Invalid version: expected 0, got %d", Version & 0xff);
    Stats.ErrorVersion++;
    return -ReadoutParser::EHEADER;
  }

  // Check cookie
  uint32_t SwappedCookie = (*(uint32_t *)(Buffer + 2)) & 0xffffff;
  XTRACE(PROCESS, DEB, "SwappedCookie 0x%08x", SwappedCookie);
  if (SwappedCookie != 0x535345) {
    XTRACE(PROCESS, WAR, "Wrong Cookie, 'ESS' expected");
    Stats.ErrorCookie++;
    return -ReadoutParser::EHEADER;
  }

  // Packet is ESS readout version 0, now we can add more header size checks
  if (Size < sizeof(PacketHeaderV0)) {
    XTRACE(PROCESS, WAR, "Invalid data size for v0 (%u)", Size);
    Stats.ErrorSize++;
    return -ReadoutParser::ESIZE;
  }

  // It is safe to cast packet header v0 struct to data
  Packet.HeaderPtr = (PacketHeaderV0 *)Buffer;

  if (Size < Packet.HeaderPtr->TotalLength or
      Packet.HeaderPtr->TotalLength < sizeof(PacketHeaderV0)) {
    XTRACE(PROCESS, WAR, "Data length mismatch, expected %u, got %u",
           Packet.HeaderPtr->TotalLength, Size);
    Stats.ErrorSize++;
    return -ReadoutParser::ESIZE;
  }

  uint8_t Type = Packet.HeaderPtr->CookieAndType >> 24;
  if ( Type!= ExpectedType) {
    XTRACE(PROCESS, WAR, "Unsupported data type (%u) for v0 (expected %u)",
           Type, ExpectedType);
    Stats.ErrorTypeSubType++;
    return -ReadoutParser::EHEADER;
  }

  if (Packet.HeaderPtr->OutputQueue >= MaxOutputQueues) {
    XTRACE(PROCESS, WAR, "Output queue %u exceeds max size %u",
           Packet.HeaderPtr->OutputQueue, MaxOutputQueues);
    Stats.ErrorOutputQueue++;
    return -ReadoutParser::EHEADER;
  }

  if (NextSeqNum[Packet.HeaderPtr->OutputQueue] != Packet.HeaderPtr->SeqNum) {
    XTRACE(PROCESS, WAR, "Bad sequence number (expected %u, got %u)",
           NextSeqNum, Packet.HeaderPtr->SeqNum);
    Stats.ErrorSeqNum++;
    NextSeqNum[Packet.HeaderPtr->OutputQueue] = Packet.HeaderPtr->SeqNum;
  }

  NextSeqNum[Packet.HeaderPtr->OutputQueue]++;
  Packet.DataPtr = (char *)(Buffer + sizeof(PacketHeaderV0));
  Packet.DataLength = Packet.HeaderPtr->TotalLength - sizeof(PacketHeaderV0);

  return ReadoutParser::OK;
}
