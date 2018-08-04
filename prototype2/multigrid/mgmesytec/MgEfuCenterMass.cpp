/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/MgEfuCenterMass.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

uint32_t MgEfuCenterMass::x() const {
  return static_cast<uint32_t>(xmass / xsum);
}

uint32_t MgEfuCenterMass::y() const {
  return static_cast<uint32_t>(ymass / ysum);
}

uint32_t MgEfuCenterMass::z() const {
  return static_cast<uint32_t>(zmass / zsum);
}

void MgEfuCenterMass::reset() {
  xmass = 0;
  ymass = 0;
  zmass = 0;

  xsum = 0;
  ysum = 0;
  zsum = 0;
}

// \todo pick only a few data points

bool MgEfuCenterMass::ingest(const MGHit& hit) {
  auto adc = mappings.rescale(hit.bus, hit.channel, hit.adc);

  if (mappings.isWire(hit.bus, hit.channel) && adc) {
    xmass += mappings.x(hit.bus, hit.channel) * adc;
    zmass += mappings.z(hit.bus, hit.channel) * adc;
    xsum += adc;
    zsum += adc;
//    DTRACE(INF, "     wire: xmass=%d, zmass=%d, xcount=%d, xmass=%d", channel);
    if (hists)
      hists->binstrips(mappings.wire(hit.bus, hit.channel), adc, 0, 0);
    return true;
  } else if (mappings.isGrid(hit.bus, hit.channel) && adc) {
    ymass += mappings.y(hit.bus, hit.channel) * adc;
    ysum += adc;
//    DTRACE(INF, "     new grid adc max: ch %d\n", channel);
    if (hists)
      hists->binstrips(0, 0, mappings.grid(hit.bus, hit.channel), adc);
    return true;
  }
  return false;
}

bool MgEfuCenterMass::event_good() const {
  return xsum && ysum && zsum;
}
