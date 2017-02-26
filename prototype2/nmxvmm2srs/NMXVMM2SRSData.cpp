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
  struct srshdr * srsptr = (struct srshdr *)buffer;

    if (size < 4) {
      XTRACE(PROCESS, DEB, "Undersize data\n");
      errbytes += size;
      return 0;
    }

    frame_counter = ntohl(srsptr->fc);
    if (frame_counter == 0xfafafafa) {
      printf("End of Frame\n");
      return -1;
    }

    if (size < 12) {
      XTRACE(PROCESS, DEB, "Undersize data II\n");
      errbytes += size;
      return 0;
    }

    dataid = ntohl(srsptr->dataid);

    if ((dataid & 0xffffff00) == 0x56413200 || size <= 12) {
      printf("No Data in packet\n");
      return 0;
    }

    if ((dataid & 0xffffff00) != 0x564d3200) {
      printf("Unknown data\n");
      errbytes += size;
      return 0;
    }

    auto datalen = size - 12;
    if ((datalen & 0xfff8) != datalen) {
      printf("Invalid data length: %d\n", datalen);
      return 0;
    }

    int chno = dataid & 0xff;
    printf("VMM2 Data, chno %d\n", chno);
    printf("Hits: %d\n", datalen/8);
    return datalen/8;
}
