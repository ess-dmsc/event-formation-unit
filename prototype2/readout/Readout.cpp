/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <libs/include/gccintel.h> // UNUSED macros
#include <readout/Readout.h>

#define CKSUMSIZE 4U

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

int Readout::validate(const char *buffer, uint32_t size) {
  if (buffer == 0) {
    XTRACE(PROCESS, WAR,
           "no buffer specified"); /**< @todo increment counter */
    return -Readout::EBUFFER;
  }

  if ((size < 64) || (size > 8960)) {
    XTRACE(PROCESS, WAR, "Invalid data size (%u)", size);
    return -Readout::ESIZE;
  }

  if (size % 64 != 0) {
    XTRACE(PROCESS, WAR,
           "data size (%u) is not padded to multiple of 64 bytes", size);
    return -Readout::EPAD;
  }

  // Add more checks - checksum, and padding

  auto hdrp = (struct Readout::Payload *)buffer;
  type = hdrp->type;
  wordcount = hdrp->wordcount;
  seqno = hdrp->seqno;
  reserved = hdrp->reserved;

  if (size < sizeof(Readout::Payload) + wordcount * 2U + CKSUMSIZE) {
    XTRACE(PROCESS, WAR,
           "Data size mismatch: size received %lu, headers and data %d",
           sizeof(Readout::Payload), CKSUMSIZE + wordcount * 2U);
    return -Readout::EHDR;
  }

  return 0;
}
