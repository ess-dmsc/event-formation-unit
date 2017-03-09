/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cinttypes>
#include <common/Trace.h>
#include <cstdio>
#include <nmxvmm2srs/NMXVMM2SRSData.h>
#include <string.h>

#define UNUSED __attribute__((unused))

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_WAR

int NMXVMM2SRSData::parse(UNUSED uint32_t data1, UNUSED uint32_t data2,
                          struct VMM2Data *vmd) {
  data1 = reversebits(data1);
  data2 = reversebits(data2);

  vmd->tdc = ((data1 >> 18) & 0x3f) + (((data1 >> 8) & 0x03) << 6);
  vmd->adc = ((data1 >> 24) & 0xff) + (((data1 >> 16) & 0x03) << 8);
  vmd->bcid = gray2bin32(((data1 >> 10) & 0x3f) + (((data1 >> 0) & 0x3f) << 6));
  vmd->chno = (data2 >> 2) & 0x3f;
  return 0;
}

int NMXVMM2SRSData::receive(const char *buffer, int size) {
  elems = 0;
  error = 0;

  if (size < 4) {
    XTRACE(PROCESS, DEB, "Undersize data\n");
    error += size;
    return 0;
  }

  struct SRSHdr *srsptr = (struct SRSHdr *)buffer;
  srshdr.fc = ntohl(srsptr->fc);
  if (srshdr.fc == 0xfafafafa) {
    XTRACE(PROCESS, DEB, "End of Frame\n");
    return -1;
  }

  if (size < 12) {
    XTRACE(PROCESS, WAR, "Undersize data II\n");
    error += size;
    return 0;
  }

  srshdr.time = ntohl(srsptr->time);
  srshdr.dataid = ntohl(srsptr->dataid);

  if (srshdr.dataid == 0x56413200) {
    XTRACE(PROCESS, DEB, "No Data\n");
    return 0;
  }

  if ((srshdr.dataid & 0xffffff00) != 0x564d3200) {
    XTRACE(PROCESS, WAR, "Unknown data\n");
    error += size;
    return 0;
  }

  if (size < 20) {
    XTRACE(PROCESS, INF, "No room for data in packet, implicit empty?\n");
    error += size;
    return 0;
  }

  auto datalen = size - 12;
  if ((datalen & 0xfff8) != datalen) {
    XTRACE(PROCESS, WAR, "Invalid data length: %d\n", datalen);
    error += size;
    return 0;
  }

  int vmmid = srshdr.dataid & 0xff;
  XTRACE(PROCESS, DEB, "VMM2 Data, VMM Id %d\n", vmmid);

  int index = 0;
  while (datalen >= 8) {
    XTRACE(PROCESS, DEB, "index: %d, datalen %d, elems: %d\n", index, datalen,
           elems);
    uint32_t data1 = htonl(*(uint32_t *)&buffer[12 + 8 * index]);
    uint32_t data2 = htonl(*(uint32_t *)&buffer[16 + 8 * index]);

    int res = parse(data1, data2, &data[index]);
    if (res == 0) {
      elems++;
      index++;
    }
    datalen -= 8;
    if (elems == max_elements && datalen >= 8) {
      XTRACE(PROCESS, DEB, "Data overflow, skipping %d bytes\n", datalen);
      break;
    }
  }
  return elems;
}

void NMXVMM2SRSData::hist_clear() {
  memset(xyhist, 0, sizeof(xyhist));
}

unsigned int NMXVMM2SRSData::gray2bin32(unsigned int num) {
  num = num ^ (num >> 16);
  num = num ^ (num >> 8);
  num = num ^ (num >> 4);
  num = num ^ (num >> 2);
  num = num ^ (num >> 1);
  return num;
}

unsigned int NMXVMM2SRSData::reversebits(register unsigned int x) {
  x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
  x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
  x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
  x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
  return ((x >> 16) | (x << 16));
}
