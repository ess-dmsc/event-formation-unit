/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/EfuCenterMass.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

uint32_t EfuCenterMass::x() const {
  return static_cast<uint32_t>(xmass / xsum);
}

uint32_t EfuCenterMass::y() const {
  return static_cast<uint32_t>(ymass / ysum);
}

uint32_t EfuCenterMass::z() const {
  return static_cast<uint32_t>(zmass / zsum);
}

uint64_t EfuCenterMass::time() const {
  return time_;
}

size_t EfuCenterMass::ingest(const std::vector<Hit> &hits) {
  this->reset();

  size_t ret{0};
  for (const auto &h : hits) {
    // \todo filter out external trigger non-events
    if (h.external_trigger)
      continue;
    if (this->ingest(h)) {
      ret++;
    }
  }
  return ret;
}

void EfuCenterMass::reset() {
  xmass = 0;
  ymass = 0;
  zmass = 0;

  xsum = 0;
  ysum = 0;
  zsum = 0;

  time_ = 0;
}

// \todo pick only a few data points

bool EfuCenterMass::ingest(const Hit &hit) {
  auto adc = mappings.rescale(hit.bus, hit.channel, hit.adc);

  if (!mappings.is_valid(hit.bus, hit.channel, adc))
    return false;

  // Pick latest time
  time_ = std::max(hit.total_time, time_);

  if (mappings.isWire(hit.bus, hit.channel) && adc) {
    xmass += mappings.x(hit.bus, hit.channel) * adc;
    zmass += mappings.z(hit.bus, hit.channel) * adc;
    xsum += adc;
    zsum += adc;
//    DTRACE(INF, "     wire: xmass=%d, zmass=%d, xcount=%d, xmass=%d", channel);
    if (raw1)
      raw1->addEntry(1, mappings.wire(hit.bus, hit.channel), hit.total_time, adc);
    if (hists)
      hists->binstrips(mappings.wire(hit.bus, hit.channel), adc, 0, 0);
    return true;
  } else if (mappings.isGrid(hit.bus, hit.channel) && adc) {
    ymass += mappings.y(hit.bus, hit.channel) * adc;
    ysum += adc;
//    DTRACE(INF, "     new grid adc max: ch %d\n", channel);
    if (raw1)
      raw1->addEntry(2, mappings.grid(hit.bus, hit.channel), hit.total_time, adc);
    if (hists)
      hists->binstrips(0, 0, mappings.grid(hit.bus, hit.channel), adc);
    return true;
  }
  return false;
}

bool EfuCenterMass::event_good() const {
  return xsum && ysum && zsum;
}

}