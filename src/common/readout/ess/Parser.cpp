// Copyright (C) 2017 - 2025 European Spallation Source, ERIC. See LICENSE file
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
#include <common/types/TimeSourceTypes.h>

#include <fmt/format.h>

#include <cstring>
#include <memory>

namespace ESSReadout {

using namespace esstime;

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

Parser::Parser(Statistics &StatsRef)
    : ESSHeaderStats(StatsRef), Packet(StatsRef) {
  std::memset(NextSeqNum, 0, sizeof(NextSeqNum));
  // All statistics counters are now registered automatically via StatCounterBase
}

int Parser::validate(const char *Buffer, uint32_t Size, uint8_t ExpectedType) {

  HeaderVersion hVersion = HeaderVersion::V0;

  if (Buffer == nullptr or Size == 0) {
    XTRACE(PROCESS, WAR, "no buffer specified");
    ESSHeaderStats.ErrorBuffer++;
    return -Parser::EBUFFER;
  }

  if ((Size < MinDataSize) || (Size > MaxUdpDataSize)) {
    XTRACE(PROCESS, WAR, "Invalid data size (%u)", Size);
    ESSHeaderStats.ErrorSize++;
    return -Parser::ESIZE;
  }

  uint32_t VersionAndPad = htons(*(uint16_t *)(Buffer));
  if ((VersionAndPad >> 8) != 0) {
    XTRACE(PROCESS, WAR, "Padding is wrong (should be 0)");
    ESSHeaderStats.ErrorPad++;
    return -Parser::EHEADER;
  }

  if ((VersionAndPad & 0xff) == HeaderVersion::V0) {
    hVersion = HeaderVersion::V0;
  } else if ((VersionAndPad & 0xff) == HeaderVersion::V1) {
    hVersion = HeaderVersion::V1;
  } else {
    XTRACE(PROCESS, WAR, "Invalid version: expected 0, got %d",
           VersionAndPad & 0xff);
    ESSHeaderStats.ErrorVersion++;
    return -Parser::EHEADER;
  }

  // Check cookie
  uint32_t SwappedCookie = (*(uint32_t *)(Buffer + 2)) & 0xffffff;
  // XTRACE(PROCESS, DEB, "SwappedCookie 0x%08x", SwappedCookie);
  if (SwappedCookie != 0x535345) {
    XTRACE(PROCESS, WAR, "Wrong Cookie, 'ESS' expected");
    ESSHeaderStats.ErrorCookie++;
    return -Parser::EHEADER;
  }

  // Check for v0 header size if the version field was v0
  if (hVersion == HeaderVersion::V0 && Size < sizeof(PacketHeaderV0)) {
    XTRACE(PROCESS, WAR, "Invalid data size for v0 (%u)", Size);
    ESSHeaderStats.ErrorSize++;
    return -Parser::ESIZE;
  }

  // Check for v1 header size if the version field was v1
  if (hVersion == HeaderVersion::V1 && Size < sizeof(PacketHeaderV1)) {
    XTRACE(PROCESS, WAR, "Invalid data size for v0 (%u)", Size);
    ESSHeaderStats.ErrorSize++;
    return -Parser::ESIZE;
  }

  switch (hVersion) {
  case HeaderVersion::V0:
    Packet.HeaderPtr = PacketHeader((PacketHeaderV0 *)(Buffer));
    ESSHeaderStats.Version0Header++;
    break;
  default:
    Packet.HeaderPtr = PacketHeader((PacketHeaderV1 *)(Buffer));
    ESSHeaderStats.Version1Header++;
    break;
  }

  if (Size != Packet.HeaderPtr.getTotalLength() or
      Packet.HeaderPtr.getTotalLength() < Packet.HeaderPtr.getSize()) {
    XTRACE(PROCESS, WAR, "Data length mismatch, expected %u, got %u",
           Packet.HeaderPtr.getTotalLength(), Size);
    ESSHeaderStats.ErrorSize++;
    return -Parser::ESIZE;
  }

  uint8_t Type = Packet.HeaderPtr.getCookieAndType() >> 24;
  if (Type != ExpectedType) {
    XTRACE(PROCESS, WAR, "Unsupported data type (%u) for v0 (expected %u)",
           Type, ExpectedType);
    ESSHeaderStats.ErrorTypeSubType++;
    return -Parser::EHEADER;
  }

  if (Packet.HeaderPtr.getOutputQueue() >= MaxOutputQueues) {
    XTRACE(PROCESS, WAR, "Output queue %u exceeds max size %u",
           Packet.HeaderPtr.getOutputQueue(), MaxOutputQueues);
    ESSHeaderStats.ErrorOutputQueue++;
    return -Parser::EHEADER;
  }

  ESSHeaderStats.OQRxPackets[Packet.HeaderPtr.getOutputQueue()]++;

  // Check per OutputQueue packet sequence number
  if (NextSeqNum[Packet.HeaderPtr.getOutputQueue()] !=
      Packet.HeaderPtr.getSeqNum()) {
    XTRACE(PROCESS, WAR, "Bad sequence number for OQ %u (expected %u, got %u)",
           Packet.HeaderPtr.getOutputQueue(),
           NextSeqNum[Packet.HeaderPtr.getOutputQueue()],
           Packet.HeaderPtr.getSeqNum());
    ESSHeaderStats.ErrorSeqNum++;
    NextSeqNum[Packet.HeaderPtr.getOutputQueue()] =
        Packet.HeaderPtr.getSeqNum();
  }

  NextSeqNum[Packet.HeaderPtr.getOutputQueue()]++;
  if (hVersion == HeaderVersion::V1) {
    Packet.DataPtr = (char *)(Buffer + sizeof(PacketHeaderV1));
    Packet.DataLength =
        Packet.HeaderPtr.getTotalLength() - sizeof(PacketHeaderV1);
  } else {
    Packet.DataPtr = (char *)(Buffer + sizeof(PacketHeaderV0));
    Packet.DataLength =
        Packet.HeaderPtr.getTotalLength() - sizeof(PacketHeaderV0);
  }

  // Check time source flags - for further details, see the ticket
  //
  //   https://jira.ess.eu/browse/ECDC-5154 for further details
  //
  const uint8_t TS = Packet.HeaderPtr.getTimeSource();
  bool TSError = false;

  if ( !(TS & TimeSource::TIME_SOURCE) && (TS & TimeSource::SYNC_SOURCE) ) {
    XTRACE(PROCESS, WAR, "Time source is local but sync source is MRF, TimeSrc = 0x%x", TS);
    ESSHeaderStats.ErrorTiming++;
    TSError = true;
  }

  if ( !(TS & TimeSource::TIME_SOURCE) && (TS & TimeSource::TIMING_STATUS) ) {
    XTRACE(PROCESS, WAR, "Time status: MRF error when time source is MRF, TimeSrc = 0x%x", TS);
    ESSHeaderStats.ErrorTimeStatus++;
    TSError = true;
  }

  if (TS & TimeSource::EVEN_FIBRE_SYNC) {
    XTRACE(PROCESS, WAR, "Even fibre has lost sync, TimeSrc = 0x%x", TS);
    ESSHeaderStats.ErrorEvenFibreSync++;
    TSError = true;
  }

  if (TS & TimeSource::ODD_FIBRE_SYNC) {
    XTRACE(PROCESS, WAR, "Odd fibre has lost sync, TimeSrc = 0x%x", TS);
    ESSHeaderStats.ErrorOddFibreSync++;
    TSError = true;
  }

  if (TSError) {
    return -Parser::EHEADER;
  }

  //
  // Check time values
  if (Packet.HeaderPtr.getPulseLow() > MaxFracTimeCount) {
    XTRACE(PROCESS, WAR, "Pulse time low (%u) exceeds max cycle count (%u)",
           Packet.HeaderPtr.getPulseLow(), MaxFracTimeCount);
    ESSHeaderStats.ErrorTimeFrac++;
    return -Parser::EHEADER;
  }

  if (Packet.HeaderPtr.getPrevPulseLow() > MaxFracTimeCount) {
    XTRACE(PROCESS, WAR,
           "Prev pulse time low (%u) exceeds max cycle count (%u)",
           Packet.HeaderPtr.getPrevPulseLow(), MaxFracTimeCount);
    ESSHeaderStats.ErrorTimeFrac++;
    return -Parser::EHEADER;
  }

  Packet.Time.setReference(
      ESSTime(Packet.HeaderPtr.getPulseHigh(), Packet.HeaderPtr.getPulseLow()));
  Packet.Time.setPrevReference(ESSTime(Packet.HeaderPtr.getPrevPulseHigh(),
                                       Packet.HeaderPtr.getPrevPulseLow()));

  XTRACE(DATA, DEB, "PulseTime     (0x%08x,0x%08x)",
         Packet.HeaderPtr.getPulseHigh(), Packet.HeaderPtr.getPulseLow());
  XTRACE(DATA, DEB, "PrevPulseTime (0x%08x,0x%08x)",
         Packet.HeaderPtr.getPrevPulseHigh(),
         Packet.HeaderPtr.getPrevPulseLow());

  if (Packet.Time.getRefTimeNS() - Packet.Time.getPrevRefTimeNS() >
      MaxPulseTimeDiffNS) {
    XTRACE(DATA, WAR,
           "PulseTime and PrevPulseTime too far apart: %" PRIu64
           ". Max allowed %u",
           (Packet.Time.getRefTimeNS() - Packet.Time.getPrevRefTimeNS()),
           MaxPulseTimeDiffNS);
    XTRACE(DATA, WAR, "PulseTimeHi      0x%08x",
           Packet.HeaderPtr.getPulseHigh());
    XTRACE(DATA, WAR, "PulseTimeLow     0x%08x",
           Packet.HeaderPtr.getPulseLow());
    XTRACE(DATA, WAR, "PulseTime (ns)   %" PRIu64 "",
           Packet.Time.getRefTimeUInt64());
    XTRACE(DATA, WAR, "PrevPulseTimeHi  0x%08x",
           Packet.HeaderPtr.getPrevPulseHigh());
    XTRACE(DATA, WAR, "PrevPulseTimeLow 0x%08x",
           Packet.HeaderPtr.getPrevPulseLow());
    XTRACE(DATA, WAR, "PrevPulseTime (ns) %" PRIu64 "",
           Packet.Time.getPrevRefTimeUInt64());
    ESSHeaderStats.ErrorTimeHigh++;

    return -Parser::EHEADER;
  }

  //
  if (hVersion == HeaderVersion::V0 &&
      Packet.HeaderPtr.getTotalLength() == sizeof(Parser::PacketHeaderV0)) {
    XTRACE(PROCESS, DEB, "Heartbeat packet v0 (pulse time only)");
    ESSHeaderStats.HeartBeats++;
  }

  if (hVersion == HeaderVersion::V1 &&
      Packet.HeaderPtr.getTotalLength() == sizeof(Parser::PacketHeaderV1)) {
    XTRACE(PROCESS, DEB, "Heartbeat packet v1 (pulse time only)");
    ESSHeaderStats.HeartBeats++;
  }

  return Parser::OK;
}

} // namespace ESSReadout