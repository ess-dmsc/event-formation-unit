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

bool MgEfuMaximum::ingest(uint8_t bus, uint16_t channel, uint16_t adc) {
  adc = mappings.rescale(bus, channel, adc);
  if (mappings.isWire(bus, channel)) {
    if (adc > WireAdcMax) {
      WireGood = true;
      WireAdcMax = adc;
      x_ = mappings.x(bus, channel);
      z_ = mappings.z(bus, channel);
      XTRACE(PROCESS, DEB, "     new wire adc max: ch %d", channel);
    }
    if (hists)
      hists->binstrips(mappings.wire(bus, channel), adc, 0, 0);
    return true;
  } else if (mappings.isGrid(bus, channel)) {
    if (adc > GridAdcMax) {
      GridGood = true;
      GridAdcMax = adc;
      y_ = mappings.y(bus, channel);
      XTRACE(PROCESS, DEB, "     new grid adc max: ch %d", channel);
    }
    if (hists)
      hists->binstrips(0, 0, mappings.grid(bus, channel), adc);
    return true;
  }
  return false;
}

bool MgEfuMaximum::event_good() const {
  return WireGood && GridGood;
}
