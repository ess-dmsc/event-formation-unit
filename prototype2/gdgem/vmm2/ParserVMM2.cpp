/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cinttypes>
#include <common/Trace.h>
#include <cstdio>
#include <gdgem/vmm2/ParserVMM2.h>
#include <libs/include/BitMath.h>
#include <string.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

int NMXVMM2SRSData::parse(uint32_t data1, uint32_t data2,
                          struct VMM2Data *vmd) {
  data1 = BitMath::reversebits32(data1);
  data2 = BitMath::reversebits32(data2);

  vmd->tdc = ((data1 >> 18) & 0x3f) + (((data1 >> 8) & 0x03) << 6);
  vmd->adc = ((data1 >> 24) & 0xff) + (((data1 >> 16) & 0x03) << 8);
  vmd->bcid = BitMath::gray2bin32(((data1 >> 10) & 0x3f) + (((data1 >> 0) & 0x3f) << 6));
  vmd->chno = (data2 >> 2) & 0x3f;
  vmd->overThreshold = (data2 >> 1) & 0x01;
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

  if ((srshdr.fc < old_frame_counter)
      && !frame_counter_overflow(old_frame_counter, srshdr.fc)) {
    XTRACE(PROCESS, WAR, "Frame counter error\n");
    error++;
  }
  old_frame_counter = srshdr.fc;

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
  XTRACE(PROCESS, DEB, "VMM2/3 Data, VMM Id %d\n", vmmid);

  int index = 0;
  while (datalen >= 8) {
    XTRACE(PROCESS, DEB, "index: %d, datalen %d, elems: %u\n", index, datalen,
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
