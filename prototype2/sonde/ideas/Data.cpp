/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <sonde/ideas/Data.h>

using namespace std;

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

/** @todo no error checking, assumes valid data and valid buffer */
int IDEASData::createevent(uint32_t time, uint32_t pixel_id, char *buffer) {

  if (pixel_id < 1) {
    XTRACE(PROCESS, WAR, "invalid pixel_id %d\n", pixel_id);
    return -1;
  }

  static_assert(sizeof(time) == 4, "time should be 32 bit");
  static_assert(sizeof(pixel_id) == 4, "pixelid should be 32 bit");

  std::memcpy(buffer + 0, &time, sizeof(time));
  std::memcpy(buffer + 4, &pixel_id, sizeof(pixel_id));
  return 0;
}

int IDEASData::receive(const char *buffer, int size) {

  if (buffer == nullptr) {
    XTRACE(PROCESS, WAR, "Invalid buffer\n");
    return 0;
  }

  if (size < 10) {
    XTRACE(PROCESS, WAR, "IDEAS readout header too short (%d bytes)\n", size);
    return 0;
  }

  struct Header * hdr = (struct Header *)buffer;

  int version = (hdr->id & 0xe000) >> 13;
  int sysno = (hdr->id & 0x1fff) >> 8;
  int type = (hdr->id & 0x00ff);
  int seqflag = (hdr->pktseq & 0xc000) >> 14;
  int count = (hdr->pktseq & 0x3fff);
  XTRACE(PROCESS, DEB, "version: %d, sysno: %d, type: %d\n", version, sysno, type);
  XTRACE(PROCESS, DEB, "sequence flag: %d, packet count: %d\n", seqflag, count);
  XTRACE(PROCESS, DEB, "time: %d, size: %d\n", hdr->timestamp, hdr->length);

  return 1; /** @todo implement data parser */
}
