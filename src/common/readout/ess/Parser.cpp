// Copyright (C) 2017 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS readout data parser implementation
///
//===----------------------------------------------------------------------===//

#include <arpa/inet.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <cstring>

namespace ESSReadout {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

Parser::Parser() { std::memset(NextSeqNum, 0, sizeof(NextSeqNum)); }

int Parser::validate(const char *Buffer, uint32_t Size, uint8_t ExpectedType) {

  HeaderVersion hVersion = HeaderVersion::V0;

  if (Buffer == nullptr or Size == 0) {
    XTRACE(PROCESS, WAR, "no buffer specified");
    Stats.ErrorBuffer++;
    return -Parser::EBUFFER;
  }

  if ((Size < MinDataSize) || (Size > MaxUdpDataSize)) {
    XTRACE(PROCESS, WAR, "Invalid data size (%u)", Size);
    Stats.ErrorSize++;
    return -Parser::ESIZE;
  }

  uint32_t VersionAndPad = htons(*(uint16_t *)(Buffer));
  if ((VersionAndPad >> 8) != 0) {
    XTRACE(PROCESS, WAR, "Padding is wrong (should be 0)");
    Stats.ErrorPad++;
    return -Parser::EHEADER;
  }

  if ((VersionAndPad & 0xff) == HeaderVersion::V0) {
    hVersion = HeaderVersion::V0;
    Stats.Version0Header++;
  } else if ((VersionAndPad & 0xff) == HeaderVersion::V1) {
    hVersion = HeaderVersion::V1;
    Stats.Version1Header++;
  } else {
    XTRACE(PROCESS, WAR, "Invalid version: expected 0, got %d",
           VersionAndPad & 0xff);
    Stats.ErrorVersion++;
    return -Parser::EHEADER;
  }

  // Check cookie
  uint32_t SwappedCookie = (*(uint32_t *)(Buffer + 2)) & 0xffffff;
  // XTRACE(PROCESS, DEB, "SwappedCookie 0x%08x", SwappedCookie);
  if (SwappedCookie != 0x535345) {
    XTRACE(PROCESS, WAR, "Wrong Cookie, 'ESS' expected");
    Stats.ErrorCookie++;
    return -Parser::EHEADER;
  }

  // Check for v0 header size if the version field was v0
  if (hVersion == HeaderVersion::V0 && Size < sizeof(PacketHeaderV0)) {
    XTRACE(PROCESS, WAR, "Invalid data size for v0 (%u)", Size);
    Stats.ErrorSize++;
    return -Parser::ESIZE;
  }

  // Check for v1 header size if the version field was v1
  if (hVersion == HeaderVersion::V1 && Size < sizeof(PacketHeaderV1)) {
    XTRACE(PROCESS, WAR, "Invalid data size for v0 (%u)", Size);
    Stats.ErrorSize++;
    return -Parser::ESIZE;
  }

  if (hVersion == HeaderVersion::V0) {
    Packet.HeaderPtr = (PacketHeaderV0 *)(Buffer);
  } else {
    Packet.HeaderPtr = (PacketHeaderV1 *)(Buffer);
  }

  if (Size != Packet.HeaderPtr->getTotalLength() or
      Packet.HeaderPtr->getTotalLength() < sizeof(PacketHeaderV0)) {
    XTRACE(PROCESS, WAR, "Data length mismatch, expected %u, got %u",
           Packet.HeaderPtr->getTotalLength(), Size);
    Stats.ErrorSize++;
    return -Parser::ESIZE;
  }

  uint8_t Type = Packet.HeaderPtr->getCookieAndType() >> 24;
  if (Type != ExpectedType) {
    XTRACE(PROCESS, WAR, "Unsupported data type (%u) for v0 (expected %u)",
           Type, ExpectedType);
    Stats.ErrorTypeSubType++;
    return -Parser::EHEADER;
  }

  if (Packet.HeaderPtr->getOutputQueue() >= MaxOutputQueues) {
    XTRACE(PROCESS, WAR, "Output queue %u exceeds max size %u",
           Packet.HeaderPtr->getOutputQueue(), MaxOutputQueues);
    Stats.ErrorOutputQueue++;
    return -Parser::EHEADER;
  }

  // Check per OutputQueue packet sequence number
  if (NextSeqNum[Packet.HeaderPtr->getOutputQueue()] != Packet.HeaderPtr->getSeqNum()) {
    XTRACE(PROCESS, WAR, "Bad sequence number for OQ %u (expected %u, got %u)",
           Packet.HeaderPtr->getOutputQueue(),
           NextSeqNum[Packet.HeaderPtr->getOutputQueue()], Packet.HeaderPtr->getSeqNum());
    Stats.ErrorSeqNum++;
    NextSeqNum[Packet.HeaderPtr->getOutputQueue()] = Packet.HeaderPtr->getSeqNum();
  }

  NextSeqNum[Packet.HeaderPtr->getOutputQueue()]++;
  if (hVersion == HeaderVersion::V1) {
    Packet.DataPtr = (char *)(Buffer + sizeof(PacketHeaderV1));
    Packet.DataLength = Packet.HeaderPtr->getTotalLength() - sizeof(PacketHeaderV1);
  } else {
    Packet.DataPtr = (char *)(Buffer + sizeof(PacketHeaderV0));
    Packet.DataLength = Packet.HeaderPtr->getTotalLength() - sizeof(PacketHeaderV0);
  }

  //
  // Check time values
  if (Packet.HeaderPtr->getPulseLow() > MaxFracTimeCount) {
    XTRACE(PROCESS, WAR, "Pulse time low (%u) exceeds max cycle count (%u)",
           Packet.HeaderPtr->getPulseLow(), MaxFracTimeCount);
    Stats.ErrorTimeFrac++;
    return -Parser::EHEADER;
  }

  if (Packet.HeaderPtr->getPrevPulseLow() > MaxFracTimeCount) {
    XTRACE(PROCESS, WAR,
           "Prev pulse time low (%u) exceeds max cycle count (%u)",
           Packet.HeaderPtr->getPrevPulseLow(), MaxFracTimeCount);
    Stats.ErrorTimeFrac++;
    return -Parser::EHEADER;
  }

  Packet.Time.setReference(Packet.HeaderPtr->getPulseHigh(),
                           Packet.HeaderPtr->getPulseLow());
  Packet.Time.setPrevReference(Packet.HeaderPtr->getPrevPulseHigh(),
                               Packet.HeaderPtr->getPrevPulseLow());

  XTRACE(DATA, DEB, "PulseTime     (0x%08x,0x%08x)",
         Packet.HeaderPtr->getPulseHigh(), Packet.HeaderPtr->getPulseLow());
  XTRACE(DATA, DEB, "PrevPulseTime (0x%08x,0x%08x)",
         Packet.HeaderPtr->getPrevPulseHigh(), Packet.HeaderPtr->getPrevPulseLow());

  if (Packet.Time.TimeInNS - Packet.Time.PrevTimeInNS > MaxPulseTimeDiffNS) {
    XTRACE(DATA, WAR,
           "PulseTime and PrevPulseTime too far apart: %" PRIu64
           ". Max allowed %u",
           (Packet.Time.TimeInNS - Packet.Time.PrevTimeInNS),
           MaxPulseTimeDiffNS);
    XTRACE(DATA, WAR, "PulseTimeHi      0x%08x", Packet.HeaderPtr->getPulseHigh());
    XTRACE(DATA, WAR, "PulseTimeLow     0x%08x", Packet.HeaderPtr->getPulseLow());
    XTRACE(DATA, WAR, "PulseTime (ns)   %" PRIu64 "", Packet.Time.TimeInNS);
    XTRACE(DATA, WAR, "PrevPulseTimeHi  0x%08x",
           Packet.HeaderPtr->getPrevPulseHigh());
    XTRACE(DATA, WAR, "PrevPulseTimeLow 0x%08x",
           Packet.HeaderPtr->getPrevPulseLow());
    XTRACE(DATA, WAR, "PrevPulseTime (ns) %" PRIu64 "",
           Packet.Time.PrevTimeInNS);
    Stats.ErrorTimeHigh++;

    return -Parser::EHEADER;
  }

  //
  if (Packet.HeaderPtr->getTotalLength() == sizeof(Parser::PacketHeaderV0)) {
    XTRACE(PROCESS, DEB, "Heartbeat packet (pulse time only)");
    Stats.HeartBeats++;
  }

  return Parser::OK;
}
} // namespace ESSReadout
