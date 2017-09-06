/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <sonde/ideas/Data.h>

using namespace std;

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_WAR

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
  int time = htonl(hdr->timestamp);
  int length = htons(hdr->length);

  XTRACE(PROCESS, DEB, "version: %d, sysno: %d, type: %d\n", version, sysno, type);
  XTRACE(PROCESS, DEB, "sequence flag: %d, packet count: %d\n", seqflag, count);
  XTRACE(PROCESS, DEB, "time: 0x%08x, size: 0x%04x\n", time, length);

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
  int nevents = *datap;
  XTRACE(PROCESS, DEB, "Number of readout events in packet: %d\n", nevents);

  if (nevents * 5 + 1 != length) {
    XTRACE(PROCESS, WAR, "Data length error: events %d (len %d), got: %d\n",
         nevents, nevents * 5 + 1, length);
    return 0;
  }

  for (int i = 0; i < nevents; i++) {
    auto timep = (uint32_t *)(datap + i*5 + 1);
    auto aschp  = (uint8_t *)(datap + i*5 + 5);
    uint32_t time = ntohl(*timep);
    uint8_t asch = *aschp; // ASIC (2b) and CHANNEL (6b)
    data[i].time = time;
    data[i].pixel_id = sondegeometry->getdetectorpixelid(0, asch);
    XTRACE(PROCESS, DEB, "event: %d, time: 0x%08x, asch: 0x%02x\n", i, time, asch);
  }

  return nevents;
}
