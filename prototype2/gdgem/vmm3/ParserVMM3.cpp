/** Copyright (C) 2018 European Spallation Source ERIC */

// #include <arpa/inet.h>
#include <cinttypes>
#include <common/Trace.h>
#include <cstdio>
#include <gdgem/vmm3/ParserVMM3.h>
#include <string.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

int VMM3SRSData::parse(uint32_t data1, uint16_t data2, struct VMM3Data *vmd) {

  XTRACE(PROCESS, DEB, "data1: 0x%08x, data2: 0x%04x\n", data1, data2);
  int dataflag = (data2 >> 15) & 0x1;

  if (dataflag) {
    /// Data
    XTRACE(PROCESS, DEB, "SRS Data\n");
    uint32_t data1r = BitMath::reversebits32(data1);
    uint16_t data2r = BitMath::reversebits16(data2);
    vmd->vmmid = (data1 >> 22) & 0x1F;
    vmd->tdc = data2r & 0xff;
    vmd->adc = (data1r >> 10) & 0x3FF;
    vmd->bcid = BitMath::gray2bin32(((data1r >> 20) & 0xFFF));
    vmd->chno = (data2r >> 8) & 0x3f;
    vmd->overThreshold = (data2r >> 1) & 0x01;
    uint8_t triggerOffset = (data1 >> 27) & 0x1F;
    vmd->triggerCounter = markers[vmd->vmmid].triggerCount + triggerOffset;

    // TODO: Test this logic
    // Affects only VMM3 but not VMM3a
    // TDC has reduced resolution due to most significant bit problem of current
    // sources (like ADC)
    if (false) {
      // TODO: use bit shifting instead?
      uint16_t tdcRebinned = (uint16_t) vmd->tdc / 8;
      vmd->tdc = tdcRebinned * 8;
    }

    return 1;
  } else {
    /// Marker
    XTRACE(PROCESS, DEB, "SRS Marker\n");
    uint8_t vmmid = (data2 & 0x1F);
    markers[vmmid].timeStamp = data1;
    markers[vmmid].triggerCount = (data2 >> 5) & 0x3FF;
    XTRACE(PROCESS, DEB, "vmmid: %d\n", vmmid);
    return 0;
  }
}

int VMM3SRSData::receive(const char *buffer, int size) {
  elems = 0;
  error = 0;

  // if (size < 4) {
  //   XTRACE(PROCESS, DEB, "Undersize data\n");
  //   error += size;
  //   return 0;
  // }

  struct SRSHdr *srsptr = (struct SRSHdr *)buffer;
  srshdr.fc = ntohl(srsptr->fc);

  if (srshdr.fc == 0xfafafafa) {
    XTRACE(PROCESS, DEB, "End of Frame\n");
    return -1;
  }

  if (size < SRSHeaderSize) {
    XTRACE(PROCESS, WAR, "Undersize data\n");
    error += size;
    return 0;
  }

  srshdr.txtime = ntohl(srsptr->txtime);
  srshdr.dataid = ntohl(srsptr->dataid);
  srshdr.fec = srshdr.dataid & 0xff;

  if (srshdr.dataid == 0x56413200) {
    XTRACE(PROCESS, DEB, "No Data\n");
    return 0;
  }

  /// maybe add a protocol error counter here
  if ((srshdr.dataid & 0xffffff00) != 0x564d3200) {
    XTRACE(PROCESS, WAR, "Unknown data\n");
    error += size;
    return 0;
  }

  if (size < 18) {
    XTRACE(PROCESS, INF, "No room for data in packet, implicit empty?\n");
    error += size;
    return 0;
  }

  auto datalen = size - SRSHeaderSize;
  if ((datalen % 6) != 0) {
    XTRACE(PROCESS, WAR, "Invalid data length: %d\n", datalen);
    error += size;
    return 0;
  }

  // XTRACE(PROCESS, DEB, "VMM3a Data, VMM Id %d\n", vmmid);

  int dataIndex = 0;
  static const int Data1Size{4};
  static const int HitAndMarkerSize{6};
  int readoutIndex = 0;
  while (datalen >= HitAndMarkerSize) {
    XTRACE(PROCESS, DEB, "readoutIndex: %d, datalen %d, elems: %u\n", readoutIndex, datalen,
           elems);
    auto Data1Offset = SRSHeaderSize + HitAndMarkerSize * readoutIndex;
    auto Data2Offset = Data1Offset + Data1Size;
    uint32_t data1 = htonl(*(uint32_t *)&buffer[Data1Offset]);
    uint16_t data2 = htons(*(uint16_t *)&buffer[Data2Offset]);

    int res = parse(data1, data2, &data[dataIndex]);
    if (res == 1) { // This was data
      elems++;
      dataIndex++;
    } else {
      timet0s++;
    }
    readoutIndex++;

    datalen -= 6;
    if (elems == max_elements && datalen > 0) {
      XTRACE(PROCESS, DEB, "Data overflow, skipping %d bytes\n", datalen);
      error += datalen;
      break;
    }
  }
  return elems;
}
