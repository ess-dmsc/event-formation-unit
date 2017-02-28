/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <nmxvmm2srs/NMXVMM2SRSData.h>
#include <arpa/inet.h>
#include <cinttypes>
#include <cstdio>
#include <common/Trace.h>

#define UNUSED __attribute__((unused))

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

int NMXVMM2SRSData::parse(UNUSED uint32_t data1, UNUSED uint32_t data2, struct VMM2Data * vmd) {
  data1 = reverse(data1);
  data2 = reverse(data2);

  vmd->tdc  = ((data1 >> 18) & 0x3f) + (((data1 >>  8) & 0x03) << 6);
  vmd->adc  = ((data1 >> 24) & 0xff) + (((data1 >> 16) & 0x03) << 8);
  vmd->bcid = grayToBinary32(((data1 >> 10) & 0x3f) + (((data1 >>  0) & 0x3f) << 6));
  vmd->chno =  (data2 >>  2) & 0x3f;
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

  struct SRSHdr * srsptr = (struct SRSHdr *)buffer;
  srshdr.fc = ntohl(srsptr->fc);
  if (srshdr.fc == 0xfafafafa) {
    printf("End of Frame\n");
    return -1;
  }

  if (size < 12) {
    XTRACE(PROCESS, DEB, "Undersize data II\n");
    error += size;
    return 0;
  }

  srshdr.time = ntohl(srsptr->time);
  srshdr.dataid = ntohl(srsptr->dataid);

  if (srshdr.dataid == 0x56413200) {
    printf("No Data\n");
    return 0;
  }

  if ((srshdr.dataid & 0xffffff00)!= 0x564d3200) {
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

  int vmmid = srshdr.dataid & 0xff;
  printf("VMM2 Data, VMM Id %d\n", vmmid);

  int index = 0;
  while (datalen > 0) {
    uint32_t data1 = htonl(*(uint32_t *)&buffer[12 + 8 * index]);
    uint32_t data2 = htonl(*(uint32_t *)&buffer[16 + 8 * index]);

    int res = parse(data1, data2, &data[index]);
    if (res == 0) {
      elems++;
      index++;
    }
    datalen -= 8;
  }
  return elems;
}


unsigned int NMXVMM2SRSData::grayToBinary32(unsigned int num) {
    num = num ^ (num >> 16);
    num = num ^ (num >> 8);
    num = num ^ (num >> 4);
    num = num ^ (num >> 2);
    num = num ^ (num >> 1);
    return num;
}


unsigned int NMXVMM2SRSData::reverse(register unsigned int x) {
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));
}
