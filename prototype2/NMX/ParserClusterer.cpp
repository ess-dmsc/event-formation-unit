/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/ParserClusterer.h>
#include <common/Trace.h>
#include <string.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#define BCIDCLOCK 25
#define TACSLOPE 125

ParserClusterer::ParserClusterer() { data.resize(4); }

void ParserClusterer::parse(unsigned int planeid, uint32_t timestamp,
                            struct NMXVMM2SRSData::VMM2Data *data,
                            size_t elements) {
  Eventlet eventlet;
  for (unsigned int i = 0; i < elements; i++) {
    XTRACE(PROCESS, DEB, "eventlet timestamp: hi 0x%08x, lo: 0x%08x\n",
           timestamp, (data[i].bcid << 16) + data[i].tdc);
    XTRACE(PROCESS, DEB, "eventlet  planeid: %d, strip: %d\n", planeid,
           data[i].chno);
    eventlet.time = data[i].bcid * BCIDCLOCK + (data[i].tdc * TACSLOPE / 255);
    eventlet.plane_id = planeid;   /**< @todo Geometry definitions */
    eventlet.strip = data[i].chno; /**< @todo Geometry definitions */
    eventlet.adc = data[i].adc;
    backlog_.insert(std::pair<uint64_t, Eventlet>(eventlet.time, eventlet));
  }
}

void ParserClusterer::parse(char *buf, size_t size) {
  Eventlet eventlet;

  size_t psize = sizeof(uint32_t) * 4;

  size_t limit = std::min(size / psize, size_t(9000 / psize));
  size_t byteidx = 0;
  for (; byteidx < limit; ++byteidx) {
    memcpy(data.data(), buf, psize);
    eventlet.read_packet(data);

    buf += psize;

    backlog_.insert(std::pair<uint64_t, Eventlet>(eventlet.time, eventlet));
  }
}

bool ParserClusterer::event_ready() {
  return (!backlog_.empty() &&
          ((backlog_.rbegin()->first - backlog_.begin()->first) > 30));
}

EventNMX ParserClusterer::get() {
  if (!event_ready())
    return EventNMX();
  EventNMX ret;
  auto latest = backlog_.begin()->first + 30;
  while (backlog_.begin()->first <= latest) {
    ret.push(backlog_.begin()->second);
    backlog_.erase(backlog_.begin());
  }
  return ret;
}
