/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <multigrid/mgmesytec/Data.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF

void MesytecData::mesytec_parse_n_words(uint32_t * buffer, int nWords) {
  uint32_t *datap = buffer;
  int wordsleft = nWords;

  // Sneak peek on time although it is last in packet
  int time = -1;
  uint32_t * tptr = (buffer + nWords - 1);
  if ((*tptr & 0xc0000000) == 0xc0000000) {
    time = *tptr & 0x3fffffff;
  }

  int module = -1;
  int readdata = -1;
  int bus = -1;
  int addr = -1;
  int adc = -1;

  while (wordsleft > 0) {
    auto datatype = *datap & 0xff000000;

    switch (datatype) {
      case MesytecData::mesytecHeader:
        readdata = *datap & 0x000003ff;
        assert(nWords > readdata);
        module = (*datap & 0x00ff0000) >> 16;
        DTRACE(INF, "Data len %d (words), module %d\n", readdata, module);
      break;

      case MesytecData::mesytecData:
        readouts++;
        bus = (*datap & 0x0f000000) >> 24;
        addr = (*datap & 0x00fff000) >> 12;
        adc = (*datap & 0x00000fff);
        #ifdef DUMPTOFILE
        mgdata.tofile("%d, %d, %d, %d\n", time, bus, addr, adc);
        #endif
        DTRACE(DEB, "%d, %d, %d, %d\n", time, bus, addr, adc);
      break;

      case MesytecData::mesytecTimeOffset:
        bus = (*datap & 0x0f000000) >> 24;
        DTRACE(INF, "Timeoffset (bus %d) %d\n", bus, (*datap & 0x0000ffff));
      break;

      default:
        if ((*datap & 0xc0000000) == 0xc0000000) {
            DTRACE(INF, "Timestamp: %d\n", *datap & 0x3fffffff);
          break;
        }

        if (*datap == 0x00000000) {
          //DTRACE(INF, "End of Data\n");
          break;
        }

        DTRACE(WAR, "Unknown: 0x%08x\n", *datap);
      break;
    }
    wordsleft--;
    datap++;
  }
}


int MesytecData::parse(const char *buffer, int size) {
  readouts = 0;

  int bytesleft = size;

  if (buffer[0] != 0x60) {
    return -error::EUNSUPP;
  }

  if (size < 19) {
    return -error::ESIZE;
  }

  uint32_t *datap = (uint32_t*)(buffer + 3);
  bytesleft -= 3;

  while (bytesleft > 16) {
    if ((*datap & 0x000000ff) != 0x58) {
      return -error::EUNSUPP;
    }
    auto len = ntohs((*datap & 0x00ffff00) >> 8);
    DTRACE(INF, "SIS 0x%08x,  Datalen %d\n", *datap, len);
    datap++;
    bytesleft -= 4;

    if ((*datap & 0xff000000) != sisBeginReadout) {
      return -error::EHEADER;
    }
    datap++;
    mesytec_parse_n_words(datap, len - 3);

    datap += (len - 3);
    bytesleft -= (len -3)*4;

    if (*datap != 0x87654321) {
      XTRACE(PROCESS, WAR, "Protocol mismatch, expected 0x87654321\n");
      return -error::EHEADER;
    }
    datap++;
    bytesleft -= 4;

    if ((*datap & 0xff000000) != sisEndReadout) {
      return -error::EHEADER;
    }
    datap++;
    bytesleft -= 4;
  }
  return error::OK;
}
