/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/MgEfuMaximum.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

void MgEfuMaximum::reset() {
  GridAdcMax = 0;
  WireAdcMax = 0;
  WireGood = false;
  GridGood = false;
  time_ = 0;
}

uint32_t MgEfuMaximum::x() const {
  return x_;
}

uint32_t MgEfuMaximum::y() const {
  return y_;
}

uint32_t MgEfuMaximum::z() const {
  return z_;
}

uint64_t MgEfuMaximum::time() const {
  return time_;
}

bool MgEfuMaximum::ingest(const MGHit& hit) {
  auto adc = mappings.rescale(hit.bus, hit.channel, hit.adc);

  // Pick latest time
  time_ = std::max(hit.total_time, time_);

  if (mappings.isWire(hit.bus, hit.channel)) {
    if (adc > WireAdcMax) {
      WireGood = true;
      WireAdcMax = adc;
      x_ = mappings.x(hit.bus, hit.channel);
      z_ = mappings.z(hit.bus, hit.channel);
      XTRACE(PROCESS, DEB, "     new wire adc max: ch %d", hit.channel);
    }
    if (hists)
      hists->binstrips(mappings.wire(hit.bus, hit.channel), adc, 0, 0);
    return true;
  } else if (mappings.isGrid(hit.bus, hit.channel)) {
    if (adc > GridAdcMax) {
      GridGood = true;
      GridAdcMax = adc;
      y_ = mappings.y(hit.bus, hit.channel);
      XTRACE(PROCESS, DEB, "     new grid adc max: ch %d", hit.channel);
    }
    if (hists)
      hists->binstrips(0, 0, mappings.grid(hit.bus, hit.channel), adc);
    return true;
  }
  return false;
}

bool MgEfuMaximum::event_good() const {
  return WireGood && GridGood;
}
