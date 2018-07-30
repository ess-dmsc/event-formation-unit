/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/MgEFU.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

MgEFU::MgEFU(std::shared_ptr<MgGeometry> mg_mappings, std::shared_ptr<Hists> h)
    : hists(h), MgMappings(mg_mappings) {
  if (!MgMappings)
    throw std::runtime_error("No valid Multigrid geometry mappings provided.");
}

void MgEFU::setWireThreshold(uint16_t low, uint16_t high) {
  wireThresholdLo = low;
  wireThresholdHi = high;
}

void MgEFU::setGridThreshold(uint16_t low, uint16_t high) {
  gridThresholdLo = low;
  gridThresholdHi = high;
}

void MgEFU::reset_maxima() {
  GridAdcMax = 0;
  WireAdcMax = 0;
  WireGood = false;
  GridGood = false;
}

bool MgEFU::ingest(uint8_t bus, uint16_t channel, uint16_t adc) {
  if (MgMappings->isWire(channel) && adc >= wireThresholdLo && adc <= wireThresholdHi) {
    if (adc > WireAdcMax) {
      WireGood = true;
      WireAdcMax = adc;
      x = MgMappings->x(bus, channel);
      z = MgMappings->z(bus, channel);
      DTRACE(INF, "     new wire adc max: ch %d\n", channel);
    }
    if (hists)
      hists->binstrips(channel, adc, 0, 0);
    return true;
  } else if (MgMappings->isGrid(channel) && adc >= gridThresholdLo && adc <= gridThresholdHi) {
    if (adc > GridAdcMax) {
      GridGood = true;
      GridAdcMax = adc;
      y = MgMappings->y(bus, channel);
      DTRACE(INF, "     new grid adc max: ch %d\n", channel);
    }
    if (hists)
      hists->binstrips(0, 0, channel, adc);
    return true;
  }
  return false;
}

bool MgEFU::event_good() const
{
  return WireGood && GridGood;
}
