/** Copyright (C) 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <common/Hists.h>
#include <multigrid/mgmesytec/DataParser.h>
#include <common/ReadoutSerializer.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

// @todo can only create a single event per UDP buffer
int MesytecData::getPixel() {
  if (gridmax < 0 || wiremax < 0) {
    return 0;
  }

  int x = mgseq.xcoord(0, wiremax);
  int y = mgseq.ycoord(gridmax);
  int z = mgseq.zcoord(wiremax);
  return mg.pixel3D(x,y,z);
}

int MesytecData::getTime() {
  return time;
}

void MesytecData::mesytec_parse_n_words(uint32_t *buffer, int nWords, NMXHists &hists, ReadoutSerializer &serializer) {
  uint32_t *datap = buffer;
  int wordsleft = nWords;

  time = -1;
  int module = -1;
  int bus = -1;
  int addr = -1;
  int adc = -1;
  int dataWords = -1;
  int time_diff = -1;
  int extended_ts = -1;

  gridmax = -1;
  wiremax = -1;
  int gridadcmax = 0;
  int wireadcmax = 0;
  int accept = 0;
  //printf("parse n words: %d\n", nWords);

  // Sneak peek on time although it is actually last in packet
  uint32_t *tptr = (buffer + nWords - 1);
  if ((*tptr & MesytecType::EndOfEvent) == MesytecType::EndOfEvent) {
    time = *tptr & 0x3fffffff;
  }

  while (wordsleft > 0) {
    auto datatype = *datap & MesytecTypeMask;

    switch (datatype) {
    case MesytecType::Header:
      dataWords = *datap & 0x000003ff;
      assert(nWords > dataWords);
      module = (*datap & 0x00ff0000) >> 16;
      DTRACE(INF, "   Header:  trigger=%d,  data len=%d (words),  module=%d\n", triggers, dataWords, module);
      break;

    case MesytecType::DataEvent1:
      bus = (*datap & 0x0f000000) >> 24;
      time_diff = (*datap & 0x0000ffff);
      DTRACE(INF, "   DataEvent1:  bus=%d,  time_diff=%d\n", bus, time_diff);
      break;

    case MesytecType::DataEvent2:
      // value in using something like getValue(Buffer, NBits, Offset) ?
      bus = (*datap & 0x0f000000) >> 24;
      addr = (*datap & 0x00fff000) >> 12; /**< channel */
      adc = (*datap & 0x00000fff);
      readouts++;

      DTRACE(INF, "   DataEvent2:  bus=%d  channel=%d  adc=%d\n", bus, addr, adc);

      accept = 0;
      if (mgseq.isWire(addr) && adc >= wireThresholdLo && adc <= wireThresholdHi) {
        accept = 1;
        if (adc > wireadcmax) {
          wiremax = addr;
          wireadcmax = adc;
          DTRACE(INF, "     new wadcmax: ch %d\n", addr);
        }
        hists.binstrips(addr, adc, 0, 0);
      } else if (mgseq.isGrid(addr) && adc >= gridThresholdLo && adc <= gridThresholdHi) {
        accept = 1;
        if (adc > gridadcmax) {
          gridmax = addr;
          gridadcmax = adc;
          DTRACE(INF, "     new gadcmax: ch %d\n", addr);
        }
        hists.binstrips(0,0, addr, adc);
      }

      if (accept) {
        //DTRACE(DEB, "   accepting %d,%d,%d,%d\n", time, bus, addr, adc);
        serializer.addEntry(0, addr, time, adc);

        if (dumptofile) {
          mgdata->tofile("%d, %d, %d, %d\n", time, bus, addr, adc);
        }
      } else {
        //DTRACE(DEB, "   discarding %d,%d,%d,%d\n", time, bus, addr, adc);
        discards++;
      }
      break;

    case MesytecType::ExtendedTimeStamp:
      extended_ts = (*datap & 0x0000ffff);
      DTRACE(INF, "   ExtendedTimeStamp: high_bits=%d\n", extended_ts);
      break;

    case MesytecType::FillDummy:
      DTRACE(DEB, "   FillDummy\n");
      break;

    default:
      if ((*datap & MesytecType::EndOfEvent) == MesytecType::EndOfEvent) {
        DTRACE(INF, "   EndOfEvent: timestamp: %d\n", *datap & 0x3fffffff);
        break;
      }

      DTRACE(WAR, "   Unknown: 0x%08x\n", *datap);
      break;
    }
    wordsleft--;
    datap++;
  }

  if (time == -1 || module == -1) {
    XTRACE(DATA, WAR, "   Warning: time or module not set\n");
    readouts = 0;
  }
}

MesytecData::error MesytecData::parse(const char *buffer, int size, NMXHists &hists, FBSerializer & fbserializer, ReadoutSerializer &serializer) {
  int bytesleft = size;
  readouts = 0;
  discards = 0;
  triggers = 0;
  geometry_errors = 0;
  events = 0;
  tx_bytes = 0;

  if (buffer[0] != 0x60) {
    return error::EUNSUPP;
  }

  if (size < 19) {
    return error::ESIZE;
  }

  uint32_t *datap = (uint32_t *)(buffer + 3);
  bytesleft -= 3;

  while (bytesleft > 16) {
    if ((*datap & 0x000000ff) != 0x58) {
      XTRACE(DATA, WAR, "expeced data value 0x58\n");
      return error::EUNSUPP;
    }

    auto len = ntohs((*datap & 0x00ffff00) >> 8);
    DTRACE(DEB, "sis3153 datawords %d\n", len);
    datap++;
    bytesleft -= 4;

    if ((*datap & 0xff000000) != SisType::BeginReadout) {
      XTRACE(DATA, WAR, "expected readout header value 0x%04x, got 0x%04x\n",
          SisType::BeginReadout, (*datap & 0xff000000));
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;
    triggers++;
    mesytec_parse_n_words(datap, len - 3, hists, serializer);

    int pixel = getPixel();
    int time  = getTime();
    DTRACE(DEB, "Event: (gridmax=%d,wiremax=%d) -->  pixel: %d, time: %d \n\n",
        gridmax, wiremax, pixel, time);
    if (pixel != 0) {
      tx_bytes += fbserializer.addevent(time, pixel);
      events++;
    } else {
      geometry_errors++;
    }

    datap += (len - 3);
    bytesleft -= (len - 3) * 4;

    if (*datap != 0x87654321) {
      XTRACE(DATA, WAR, "Protocol mismatch, expected 0x87654321\n");
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;

    if ((*datap & 0xff000000) != SisType::EndReadout) {
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;
  }
  // printf("bytesleft %d\n", bytesleft);
  return error::OK;
}
