/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/MgEfuMaximum.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

void MgEfuMaximum::setWireThreshold(uint16_t low, uint16_t high) {
  wireThresholdLo = low;
  wireThresholdHi = high;
}

void MgEfuMaximum::setGridThreshold(uint16_t low, uint16_t high) {
  gridThresholdLo = low;
  gridThresholdHi = high;
}

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
  if (mappings.isWire(bus, channel) && adc >= wireThresholdLo && adc <= wireThresholdHi) {
    if (adc > WireAdcMax) {
      WireGood = true;
      WireAdcMax = adc;
      x_ = mappings.x(bus, channel);
      z_ = mappings.z(bus, channel);
      DTRACE(INF, "     new wire adc max: ch %d\n", channel);
    }
    if (hists)
      hists->binstrips(mappings.wire(bus, channel), adc, 0, 0);
    return true;
  } else if (mappings.isGrid(bus, channel) && adc >= gridThresholdLo && adc <= gridThresholdHi) {
    if (adc > GridAdcMax) {
      GridGood = true;
      GridAdcMax = adc;
      y_ = mappings.y(bus, channel);
      DTRACE(INF, "     new grid adc max: ch %d\n", channel);
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
