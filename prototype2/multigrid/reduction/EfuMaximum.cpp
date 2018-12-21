/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/reduction/EfuMaximum.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

size_t EfuMaximum::ingest(const std::vector<MesytecReadout> &hits) {
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

void EfuMaximum::reset() {
  GridAdcMax = 0;
  WireAdcMax = 0;
  WireGood = false;
  GridGood = false;
  time_ = 0;
}

uint32_t EfuMaximum::x() const {
  return x_;
}

uint32_t EfuMaximum::y() const {
  return y_;
}

uint32_t EfuMaximum::z() const {
  return z_;
}

uint64_t EfuMaximum::time() const {
  return time_;
}

bool EfuMaximum::ingest(const MesytecReadout &hit) {
  auto adc = mappings.rescale(hit.bus, hit.channel, hit.adc);

  if (!mappings.is_valid(hit.bus, hit.channel, adc))
    return false;

  // Pick latest time
  time_ = std::max(hit.total_time, time_);

  if (mappings.isWire(hit.bus, hit.channel)) {
    if (adc >= WireAdcMax) {
      WireGood = true;
      WireAdcMax = adc;
      x_ = mappings.x(hit.bus, hit.channel);
      z_ = mappings.z(hit.bus, hit.channel);
      XTRACE(PROCESS, DEB, "     new wire adc max: ch %d", hit.channel);
    }
    if (raw1)
      raw1->addEntry(1, mappings.wire(hit.bus, hit.channel), hit.total_time, adc);
    if (hists)
      hists->bin_x(mappings.wire(hit.bus, hit.channel), adc);
    return true;
  } else if (mappings.isGrid(hit.bus, hit.channel)) {
    if (adc >= GridAdcMax) {
      GridGood = true;
      GridAdcMax = adc;
      y_ = mappings.y(hit.bus, hit.channel);
      XTRACE(PROCESS, DEB, "     new grid adc max: ch %d", hit.channel);
    }
    if (raw1)
      raw1->addEntry(2, mappings.grid(hit.bus, hit.channel), hit.total_time, adc);
    if (hists)
      hists->bin_y(mappings.grid(hit.bus, hit.channel), adc);
    return true;
  }
  return false;
}

bool EfuMaximum::event_good() const {
  return WireGood && GridGood;
}

}