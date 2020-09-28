/** Copyright (C) 2018 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cinttypes>
#include <cstdio>
#include <string.h>
#include <common/Trace.h>
#include <gdgem/srs/ParserVMM3.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Gem {

int ParserVMM3::parse(uint32_t data1, uint16_t data2, struct VMM3Data *vd) {
  XTRACE(PROCESS, DEB, "data1: 0x%08x, data2: 0x%04x", data1, data2);
  int dataflag = (data2 >> 15) & 0x1;

  if (dataflag) {
    /// Data
    XTRACE(PROCESS, DEB, "SRS Data");

    vd->overThreshold = (data2 >> 14) & 0x01;
    stats.ParserOverThreshold += vd->overThreshold;
    vd->chno = (data2 >> 8) & 0x3f;
    vd->tdc = data2 & 0xff;
    vd->vmmid = (data1 >> 22) & 0x1F;
    vd->triggerOffset = (data1 >> 27) & 0x1F;
    uint16_t idx = (pd.fecId - 1) * MaxVMMs + vd->vmmid;
    if(vd->triggerOffset < markers[idx].lastTriggerOffset) {
      if(markers[idx].calcTimeStamp != 0) {
        markers[idx].calcTimeStamp +=32*srsTime.trigger_period_ns()/SRSTime::internal_SRS_clock_period_ns;
      }
      if(markers[idx].fecTimeStamp != markers[idx].calcTimeStamp){
        stats.ParserTimestampLostErrors++;
        XTRACE(PROCESS, WAR, "ParserTimestampLostErrors: fc %d vmm %d: fec ts %llu, calc ts %llu, diff %f",
          pd.nextFrameCounter-1, vd->vmmid, markers[idx].fecTimeStamp,
          markers[idx].calcTimeStamp, (double)markers[idx].fecTimeStamp-
          (double)markers[idx].calcTimeStamp);
      }
    }
    markers[idx].lastTriggerOffset = vd->triggerOffset;
    vd->adc = (data1 >> 12) & 0x3FF;
    vd->bcid = BitMath::gray2bin32(data1 & 0xFFF);
    //Maybe here use the calculated timestamp instead
    //vd->fecTimeStamp = markers[idx].calcTimeStamp;
    vd->fecTimeStamp = markers[idx].fecTimeStamp;
    if(vd->fecTimeStamp > 0) {
      markers[idx].hasDataMarker = true;
      vd->hasDataMarker = true;
    }
    return 1;
  } else {
    /// Marker
    uint8_t vmmid = (data2 >> 10) & 0x1F;
    uint16_t idx = (pd.fecId - 1) * MaxVMMs + vmmid;
    uint64_t timestamp_lower_10bit = data2 & 0x03FF;
    uint64_t timestamp_upper_32bit = data1;

    uint64_t timestamp_42bit = (timestamp_upper_32bit << 10)
        + timestamp_lower_10bit;
    XTRACE(PROCESS, DEB, "SRS Marker vmmid %d: timestamp lower 10bit %u, timestamp upper 32 bit %u, 42 bit timestamp %"
        PRIu64"", vmmid, timestamp_lower_10bit, timestamp_upper_32bit, timestamp_42bit);

    if(markers[idx].fecTimeStamp > timestamp_42bit) {
      if (markers[idx].fecTimeStamp < 0x1FFFFFFF + timestamp_42bit) {
        stats.ParserTimestampSeqErrors++;
        XTRACE(PROCESS, DEB, "ParserTimestampSeqErrors:  fc %d, ts %llu, marker ts %llu", timestamp_42bit, markers[idx].fecTimeStamp);
      }
      else {
        stats.ParserTimestampOverflows++;
      }
    }
    if(markers[idx].calcTimeStamp == 0) {
      markers[idx].calcTimeStamp = timestamp_42bit;
    }
    markers[idx].fecTimeStamp = timestamp_42bit;
    return 0;
  }
}


int ParserVMM3::receive(const char *buffer, int size) {
  int hits = 0;
  if (size < 4) {
    XTRACE(PROCESS, WAR, "Undersize data");
    stats.ParserErrorBytes += size;
    stats.ParserBadFrames++;
    return 0;
  }

  struct SRSHeader *srsHeaderPtr = (struct SRSHeader *) buffer;
  hdr.frameCounter = ntohl(srsHeaderPtr->frameCounter);

  if (pd.nextFrameCounter != hdr.frameCounter) {
    if(hdr.frameCounter > pd.nextFrameCounter) {
      if(stats.ParserGoodFrames > 0) {
        stats.ParserFrameMissingErrors +=
         (hdr.frameCounter - pd.nextFrameCounter);
        XTRACE(PROCESS, WAR, "ParserFrameMissingErrors: fc %d, next fc %d",
        hdr.frameCounter, pd.nextFrameCounter);
      }
    }
    else {
      if (pd.nextFrameCounter - hdr.frameCounter > 0x0FFFFFFF) {
        stats.ParserFramecounterOverflows++;
        XTRACE(PROCESS, DEB, "ParserFramecounterOverflows: fc %d, next fc %d",
          hdr.frameCounter, pd.nextFrameCounter);
      }
      else {
        stats.ParserFrameSeqErrors++;
        XTRACE(PROCESS, WAR, "ParserFrameSeqErrors: fc %d, next fc %d",
          hdr.frameCounter, pd.nextFrameCounter);

        for(int vmmid=0; vmmid < MaxVMMs; vmmid++) {
          markers[(pd.fecId - 1) * MaxVMMs + vmmid].fecTimeStamp = 0;
          markers[(pd.fecId - 1) * MaxVMMs + vmmid].calcTimeStamp = 0;
        }
      }
    }
  }
  else {
    if(hdr.frameCounter == 0) {
      stats.ParserFramecounterOverflows++;
    }

  }
  pd.nextFrameCounter = hdr.frameCounter + 1;

  if (size < SRSHeaderSize + HitAndMarkerSize) {
    XTRACE(PROCESS, WAR, "Undersize data");
    stats.ParserBadFrames++;
    stats.ParserErrorBytes += size;
    return 0;
  }

  hdr.dataId = ntohl(srsHeaderPtr->dataId);
  /// maybe add a protocol error counter here
  if ((hdr.dataId & 0xffffff00) != 0x564d3300) {
    XTRACE(PROCESS, WAR, "Unknown data");
    stats.ParserBadFrames++;
    stats.ParserErrorBytes += size;
    return 0;
  }

  pd.fecId = (hdr.dataId >> 4) & 0x0f;
  if (pd.fecId < 1 || pd.fecId > 16) {
    XTRACE(PROCESS, WAR, "Invalid fecId: %u", pd.fecId);
    stats.ParserBadFrames++;
    stats.ParserErrorBytes += size;
    return 0;
  }
  hdr.udpTimeStamp = ntohl(srsHeaderPtr->udpTimeStamp);
  //This header component will vanish soon
  //and be replaced by a timestamp for each vmm
  hdr.offsetOverflow = ntohl(srsHeaderPtr->offsetOverflow);

  auto datalen = size - SRSHeaderSize;
  if ((datalen % 6) != 0) {
    XTRACE(PROCESS, WAR, "Invalid data length: %d", datalen);
    stats.ParserBadFrames++;
    stats.ParserErrorBytes += size;
    return 0;
  }

  int dataIndex = 0;
  int readoutIndex = 0;
  while (datalen >= HitAndMarkerSize) {
    XTRACE(PROCESS, DEB, "readoutIndex: %d, datalen %d",
    		readoutIndex, datalen);
    auto Data1Offset = SRSHeaderSize + HitAndMarkerSize * readoutIndex;
    auto Data2Offset = Data1Offset + Data1Size;
    uint32_t data1 = htonl(*(uint32_t *) &buffer[Data1Offset]);
    uint16_t data2 = htons(*(uint16_t *) &buffer[Data2Offset]);

    int res = parse(data1, data2, &data[dataIndex]);
    if (res == 1) { // This was data
      hits++;
      stats.ParserData++;
      dataIndex++;
    } else {
      stats.ParserMarkers++;
    }
    stats.ParserReadouts++;
    readoutIndex++;

    datalen -= 6;
    if (hits == maxHits && datalen > 0) {
      XTRACE(PROCESS, WAR, "Data overflow, skipping %d bytes", datalen);
      stats.ParserErrorBytes += datalen;
      break;
    }
  }
  stats.ParserGoodFrames++;

  return hits;
}

}
