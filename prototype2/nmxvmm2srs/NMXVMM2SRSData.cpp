/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <nmxvmm2srs/NMXVMM2SRSData.h>
#include <arpa/inet.h>
#include <cinttypes>
#include <cstdio>
#include <common/Trace.h>

struct srshdr {
  uint32_t fc;
  uint32_t dataid;
  uint32_t time;
};

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB


int NMXVMM2SRSData::receive(const char *buffer, int size) {

  elems = 0;
  error = 0;

  if (size < 4) {
    XTRACE(PROCESS, DEB, "Undersize data\n");
    error += size;
    return 0;
  }

  struct srshdr * srsptr = (struct srshdr *)buffer;
  frame_counter = ntohl(srsptr->fc);
  if (frame_counter == 0xfafafafa) {
    printf("End of Frame\n");
    return -1;
  }

  if (size < 12) {
    XTRACE(PROCESS, DEB, "Undersize data II\n");
    error += size;
    return 0;
  }

  uint32_t dataid = ntohl(srsptr->dataid);

  if (dataid == 0x56413200) {
    printf("No Data\n");
    return 0;
  }

  if ((dataid & 0xffffff00)!= 0x564d3200) {
    printf("Unknown data\n");
    error += size;
    return 0;
  }


  if (size < 20) {
    printf("No room for data in packet\n");
    error += size;
    return 0;
  }

  auto datalen = size - 12;
  if ((datalen & 0xfff8) != datalen) {
    printf("Invalid data length: %d\n", datalen);
    error += size;
    return 0;
  }

  int chno = dataid & 0xff;
  printf("VMM2 Data, chno %d\n", chno);
  printf("Hits: %d\n", datalen/8);
  elems = datalen/8;
  return elems;
}
