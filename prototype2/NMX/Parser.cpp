/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/Parser.h>
#include <string.h>
#include <common/Trace.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#define BCIDCLOCK 25
#define TACSLOPE 125


std::vector<Eventlet> Parser::parse(unsigned int planeid, uint32_t timestamp, struct NMXVMM2SRSData::VMM2Data * data, size_t elements) {
  std::vector<Eventlet> ret(elements);
  for (unsigned int i = 0; i < elements; i++) {
    XTRACE(PROCESS, DEB, "eventlet timestamp: hi 0x%08x, lo: 0x%08x\n", timestamp, (data[i].bcid << 16) +  data[i].tdc);
    XTRACE(PROCESS, DEB, "eventlet  planeid: %d, strip: %d\n", planeid, data[i].chno);
    ret[i].time = data[i].bcid * BCIDCLOCK +  (data[i].tdc*TACSLOPE/255);
    ret[i].plane_id = planeid;   /**< @todo Geometry definitions */
    ret[i].strip = data[i].chno; /**< @todo Geometry definitions */
    ret[i].adc = data[i].adc;
  }
  return ret;
}

std::vector<Eventlet> Parser::parse(char *buf, size_t size) {
  size_t psize = sizeof(uint32_t) * 4;
  std::vector<uint32_t> data(4);

  size_t limit = std::min(size / psize, size_t(9000 / psize));
  std::vector<Eventlet> ret(limit);

  for (size_t i=0; i < limit; ++i) {
    memcpy(data.data(), buf, psize);
    ret[i].read_packet(data);
    buf += psize;
  }
  return ret;
}