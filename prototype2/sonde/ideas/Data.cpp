/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <sonde/ideas/Data.h>

using namespace std;

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

int IDEASData::receive(const char *buffer, int size) {

  if (buffer == nullptr) {
    XTRACE(PROCESS, WAR, "Invalid buffer\n");
    return 0;
  }

  if (size < 11) {
    XTRACE(PROCESS, WAR, "IDEAS readout header too short (%d bytes)\n", size);
    return 0;
  }

  struct Header * hdr = (struct Header *)buffer;

  int version = (htons(hdr->id) & 0xe000) >> 13;
  if (version != 0) {
    XTRACE(PROCESS, WAR, "Illegal version number (%d)\n", version);
    return 0;
  }
  int sysno = (htons(hdr->id) & 0x1fff) >> 8;
  int type = (htons(hdr->id) & 0x00ff);
  int seqflag = (htons(hdr->pktseq) & 0xc000) >> 14;
  int count = (htons(hdr->pktseq) & 0x3fff);
  int hdrtime = htonl(hdr->timestamp);
  int length = htons(hdr->length);

  XTRACE(PROCESS, DEB, "version: %d, sysno: %d, type: %d\n", version, sysno, type);
  XTRACE(PROCESS, DEB, "sequence flag: %d, packet count: %d\n", seqflag, count);
  XTRACE(PROCESS, DEB, "time: 0x%08x, size: 0x%04x\n", hdrtime, length);

  if (type != 0xD6) {
    XTRACE(PROCESS, WAR, "Unsupported readout format: Expected 0xD6, got 0x%02x\n", type);
    return 0;
  }

  if (length + (int)sizeof(struct Header) != size) {
    XTRACE(PROCESS, WAR, "Packet length mismatch: udp: %d, parsed: %d\n",
         size, length + (int)sizeof(struct Header));
    return 0;
  }

  uint8_t * datap = (uint8_t *)(buffer + sizeof(struct Header));
  int nentries = *datap;
  XTRACE(PROCESS, DEB, "Number of readout events in packet: %d\n", nentries);

  if (nentries * 5 + 1 != length) {
    XTRACE(PROCESS, WAR, "Data length error: events %d (len %d), got: %d\n",
         nentries, nentries * 5 + 1, length);
    return 0;
  }

  events = 0;
  errors = 0;
  for (int i = 0; i < nentries; i++) {
    auto timep = (uint32_t *)(datap + i*5 + 1);
    auto aschp  = (uint8_t *)(datap + i*5 + 5);
    uint32_t time = ntohl(*timep);
    uint8_t asch = *aschp; // ASIC (2b) and CHANNEL (6b)

    int pixelid = sondegeometry->getdetectorpixelid(0, asch);
    if (pixelid >= 1) {
      data[events].time = time;
      data[events].pixel_id = static_cast<uint32_t>(pixelid);
      #ifdef DUMPTOFILE
          dprintf(fd, "%d, %d, %d, %d, %d, %d\n", count, hdrtime, sysno, asch>>6, asch & 0x3f, pixelid);
      #endif
      XTRACE(PROCESS, DEB, "event: %d, time: 0x%08x, pixel: %d\n", i, time, data[events].pixel_id);
      events++;
    } else {
      XTRACE(PROCESS, WAR, "Geometry error in entry %d (asch %d)\n", i, asch);
      errors++;
    }

  }
  XTRACE(PROCESS, DEB, "Number of events in buffer: %d\n", events);
  return events;
}
