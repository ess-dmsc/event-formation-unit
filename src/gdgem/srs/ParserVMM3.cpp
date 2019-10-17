/** Copyright (C) 2018 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cinttypes>
#include <common/Trace.h>
#include <cstdio>
#include <gdgem/srs/ParserVMM3.h>
#include <string.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_WAR

namespace Gem {

int VMM3SRSData::parse(uint32_t data1, uint16_t data2, struct VMM3Data *vmd) {

  XTRACE(PROCESS, DEB, "data1: 0x%08x, data2: 0x%04x", data1, data2);
  int dataflag = (data2 >> 15) & 0x1;

  if (dataflag) {
    /// Data
    XTRACE(PROCESS, DEB, "SRS Data");

    vmd->overThreshold = (data2 >> 14) & 0x01;
    vmd->chno = (data2 >> 8) & 0x3f;
    vmd->tdc = data2 & 0xff;

    vmd->vmmid = (data1 >> 22) & 0x1F;
    vmd->triggerOffset = (data1 >> 27) & 0x1F;
    vmd->adc = (data1 >> 12) & 0x3FF;
    vmd->bcid = BitMath::gray2bin32(data1 & 0xFFF);

    vmd->fecTimeStamp = markers[(parserData.fecId - 1) * maxVMMs + vmd->vmmid].fecTimeStamp;
    if (markers[(parserData.fecId - 1) * maxVMMs + vmd->vmmid].fecTimeStamp > 0) {
      vmd->hasDataMarker = true;
    }
    return 1;
  } else {
    /// Marker
    uint8_t vmmid = (data2 >> 10) & 0x1F;
    uint64_t timestamp_lower_10bit = data2 & 0x03FF;
    uint64_t timestamp_upper_32bit = data1;

    uint64_t timestamp_42bit = (timestamp_upper_32bit << 10)
        + timestamp_lower_10bit;
    XTRACE(PROCESS, DEB, "SRS Marker vmmid %d: timestamp lower 10bit %u, timestamp upper 32 bit %u, 42 bit timestamp %"
        PRIu64
        "", vmmid, timestamp_lower_10bit, timestamp_upper_32bit, timestamp_42bit);
    if(markers[(parserData.fecId - 1) * maxVMMs + vmmid].fecTimeStamp 
    > timestamp_42bit) {
      if (markers[(parserData.fecId - 1) * maxVMMs + vmmid].fecTimeStamp 
      < 0x1FFFFFFF + timestamp_42bit) {
        stats.timestamp_seq_errors++;
      }
      else {
        stats.timestamp_overflows++;
      }
    } 
    markers[(parserData.fecId - 1) * maxVMMs + vmmid].fecTimeStamp = timestamp_42bit;
    markers[(parserData.fecId - 1) * maxVMMs + vmmid].updatedInFrame = true;
    //XTRACE(PROCESS, DEB, "vmmid: %d", vmmid);
    return 0;
  }
}

int VMM3SRSData::receive(const char *buffer, int size) {
  memset(&stats, 0, sizeof(stats));

  if (size < 4) {
    XTRACE(PROCESS, DEB, "Undersize data");
    stats.errors += size;
    return 0;
  }

  struct SRSHdr *srsHeaderPtr = (struct SRSHdr *) buffer;
  srsHeader.frameCounter = ntohl(srsHeaderPtr->frameCounter);
  /*
  Does not exist anymore
  if (srsHeader.frameCounter == 0xfafafafa) {
    //XTRACE(PROCESS, INF, "End of Frame");
    stats.badFrames++;
    stats.errors += size;
    return -1;
  }
 */

  if (parserData.nextFrameCounter != srsHeader.frameCounter) {
    stats.frame_seq_errors++;
    XTRACE(PROCESS, WAR, "FC error %u != %u", parserData.nextFrameCounter, srsHeader.frameCounter);
  }

  if(srsHeader.frameCounter < 0xFFFFFFFF) {
    parserData.nextFrameCounter = srsHeader.frameCounter + 1;
  }
  else {
    stats.framecounter_overflows++;
    parserData.nextFrameCounter = 0;
  }

  
  if (size < SRSHeaderSize + HitAndMarkerSize) {
    XTRACE(PROCESS, WAR, "Undersize data");
    stats.badFrames++;
    stats.errors += size;
    return 0;
  }

  srsHeader.dataId = ntohl(srsHeaderPtr->dataId);
  /// maybe add a protocol error counter here
  if ((srsHeader.dataId & 0xffffff00) != 0x564d3300) {
    XTRACE(PROCESS, WAR, "Unknown data");
    stats.badFrames++;
    stats.errors += size;
    return 0;
  }

  parserData.fecId = (srsHeader.dataId >> 4) & 0x0f;
  if (parserData.fecId < 1 || parserData.fecId > 16) {
    XTRACE(PROCESS, WAR, "Invalid fecId: %u", parserData.fecId);
    stats.badFrames++;
    stats.errors += size;
    return 0;
  }
  srsHeader.udpTimeStamp = ntohl(srsHeaderPtr->udpTimeStamp);

  srsHeader.offsetOverflow = ntohl(srsHeaderPtr->offsetOverflow);
  for(int id=0;id<maxVMMs;id++) {
    bool timestampSent = 
    static_cast<bool>((srsHeader.offsetOverflow >> id) & 0x00000001);
    if(timestampSent != markers[(parserData.fecId - 1) * maxVMMs + id].updatedInFrame) {
      stats.timestamp_lost_errors++;  
    }
    markers[(parserData.fecId - 1) * maxVMMs + id].updatedInFrame = false;
  }

  auto datalen = size - SRSHeaderSize;
  if ((datalen % 6) != 0) {
    XTRACE(PROCESS, WAR, "Invalid data length: %d", datalen);
    stats.badFrames++;
    stats.errors += size;
    return 0;
  }

  // XTRACE(PROCESS, DEB, "VMM3a Data, VMM Id %d", vmmid);

  int dataIndex = 0;
  int readoutIndex = 0;
  while (datalen >= HitAndMarkerSize) {
    //XTRACE(PROCESS, DEB, "readoutIndex: %d, datalen %d, elems: %u",
    //		readoutIndex, datalen, stats.readouts);
    auto Data1Offset = SRSHeaderSize + HitAndMarkerSize * readoutIndex;
    auto Data2Offset = Data1Offset + Data1Size;
    uint32_t data1 = htonl(*(uint32_t *) &buffer[Data1Offset]);
    uint16_t data2 = htons(*(uint16_t *) &buffer[Data2Offset]);

    int res = parse(data1, data2, &data[dataIndex]);
    if (res == 1) { // This was data
      stats.readouts++;
      dataIndex++;
    } else {
      stats.markers++;
    }
    readoutIndex++;

    datalen -= 6;
    if (stats.readouts == maxHits && datalen > 0) {
      XTRACE(PROCESS, INF, "Data overflow, skipping %d bytes", datalen);
      stats.errors += datalen;
      break;
    }
  }
  stats.goodFrames++;

  return stats.readouts;
}

}
