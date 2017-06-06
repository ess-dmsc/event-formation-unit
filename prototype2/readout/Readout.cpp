/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <libs/include/gccintel.h>  // UNUSED macros
#include <readout/Readout.h>

#define CKSUMSIZE 4U

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

int Readout::receive(const char *buffer, int size) {
  if (buffer == 0) {
      XTRACE(PROCESS, WAR, "Zero length data\n"); /**< @todo increment counter */
      return 0;
  }

  if (size <=0) {
    XTRACE(PROCESS, WAR, "invalid data size (%d)\n", size);
    return 0;
  }

  if (size % 64 != 0) {
    XTRACE(PROCESS, WAR, "data size (%d) is not padded to multiple of 64 bytes\n", size);
    return 0;
  }

  auto hdrp = (struct Readout::Payload *)buffer;
  type = hdrp->type;
  wordcount = hdrp->wordcount;
  seqno = hdrp->seqno;
  if ((unsigned long)size < sizeof(Readout::Payload) + CKSUMSIZE + 2 * wordcount) {
    XTRACE(PROCESS, WAR, "Data size mismatch: size received %lu, headers and data %d\n",
           sizeof(Readout::Payload), CKSUMSIZE + wordcount * 2U);
    return 0;
  }
  reserved = hdrp->reserved;

  return 0;
}
