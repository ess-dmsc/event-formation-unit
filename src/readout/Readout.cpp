/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

//#include <common/gccintel.h> // UNUSED macros
#include <algorithm>
#include <common/Trace.h>
#include <readout/Readout.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

const unsigned int MaxUdpDataSize{8972};
const unsigned int MinDataSize{4}; // just cookie and version

int Readout::validate(const char *Buffer, uint32_t Size, uint8_t Type) {
  std::memset(&Packet, 0, sizeof(Packet));

  if (Buffer == nullptr or Size == 0) {
    XTRACE(PROCESS, WAR, "no buffer specified");
    Stats.ErrorBuffer++;
    return -Readout::EBUFFER;
  }

  if ((Size < MinDataSize) || (Size > MaxUdpDataSize)) {
    XTRACE(PROCESS, WAR, "Invalid data size (%u)", Size);
    Stats.ErrorSize++;
    return -Readout::ESIZE;
  }

  uint32_t CookieVer = *(uint32_t *)Buffer;
  // 'E', 'S', 'S', 0x00 - cookie + version 0
  if (CookieVer != 0x00535345) { // 'ESS0' little endian
    Stats.ErrorVersion++;
    return -Readout::EHEADER;
  }

  // Packet is ESS readout version 0,
  // now we can add more header size checks
  if (Size < sizeof(PacketHeaderV0)) {
    XTRACE(PROCESS, WAR, "Invalid data size for v0 (%u)", Size);
    Stats.ErrorSize++;
    return -Readout::ESIZE;
  }

  // Is is safe to cast packet header v0 strut to data
  Packet.HeaderPtr = (PacketHeaderV0 *)Buffer;

  if (Size < Packet.HeaderPtr->TotalLength or
      Packet.HeaderPtr->TotalLength < sizeof(PacketHeaderV0)) {
    XTRACE(PROCESS, WAR, "Data length mismatch, expected %u, got %u",
           Packet.HeaderPtr->TotalLength, Size);
    Stats.ErrorSize++;
    return -Readout::ESIZE;
  }

  if (Packet.HeaderPtr->TypeSubType != Type) {
    XTRACE(PROCESS, WAR, "Unsupported data type for v0 (%u)", Type);
    Stats.ErrorTypeSubType++;
    return -Readout::EHEADER;
  }

  if (NextSeqNum != Packet.HeaderPtr->SeqNum) {
    XTRACE(PROCESS, WAR, "Bad sequence number (expected %u, got %u)",
           NextSeqNum, Packet.HeaderPtr->SeqNum);
    Stats.ErrorSeqNum++;
    NextSeqNum = Packet.HeaderPtr->SeqNum;
  }

  NextSeqNum++;
  Packet.DataPtr = (char *)(Buffer + sizeof(PacketHeaderV0));
  Packet.DataLength = Packet.HeaderPtr->TotalLength - sizeof(PacketHeaderV0);

  return Readout::OK;
}
