/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <common/gccintel.h> // UNUSED macros
#include <readout/Readout.h>

#define CKSUMSIZE 4U

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

const unsigned int MaxUdpDataSize{8972};
const unsigned int MinDataSize{4}; // just cookie and version

int Readout::validate(const char *Buffer, uint32_t Size, uint8_t UNUSED Type) {
  if (Buffer == nullptr) {
    XTRACE(PROCESS, WAR,
           "no buffer specified"); /**< \todo increment counter */
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

  // Now we can add more header size checks
  if (Size < sizeof(Readout::PacketHeaderV0)) {
    XTRACE(PROCESS, WAR, "Invalid data size for v0 (%u)", Size);
    Stats.ErrorSize++;
    return -Readout::ESIZE;
  }

  auto HeaderPtr = (Readout::PacketHeaderV0 *)Buffer;

  if (HeaderPtr->TypeSubType != Type) {
    XTRACE(PROCESS, WAR, "Unsupported data type for v0 (%u)", Type);
    Stats.ErrorTypeSubType++;
    return -Readout::EHEADER;
  }


  return Readout::OK;
}
