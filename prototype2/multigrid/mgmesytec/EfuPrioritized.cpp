/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/EfuPrioritized.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

uint32_t EfuPrioritized::x() const {
  return static_cast<uint32_t>(xmass / xsum);
}

uint32_t EfuPrioritized::y() const {
  return static_cast<uint32_t>(ymass / ysum);
}

uint32_t EfuPrioritized::z() const {
  return static_cast<uint32_t>(zmass / zsum);
}

uint64_t EfuPrioritized::time() const {
  return time_;
}

size_t EfuPrioritized::ingest(const std::vector<Hit> &hits) {
  this->reset();

  std::vector<Hit> wires;
  std::vector<Hit> grids;

  for (auto h : hits) {
    time_ = std::max(h.total_time, time_);
    if (h.external_trigger)
      continue;
    h.adc = mappings.rescale(h.bus, h.channel, h.adc);
    if (!mappings.is_valid(h.bus, h.channel, h.adc) || !h.adc)
      continue;

    if (mappings.isWire(h.bus, h.channel)) {
      if (raw1)
        raw1->addEntry(1, mappings.wire(h.bus, h.channel), h.total_time, h.adc);
      if (hists)
        hists->binstrips(mappings.wire(h.bus, h.channel), h.adc, 0, 0);
      wires.push_back(h);
    }
    else if (mappings.isGrid(h.bus, h.channel)) {
      if (raw1)
        raw1->addEntry(2, mappings.grid(h.bus, h.channel), h.total_time, h.adc);
      if (hists)
        hists->binstrips(0, 0, mappings.grid(h.bus, h.channel), h.adc);
      grids.push_back(h);
    }
  }

  size_t ret{0};

  if (!wires.empty())
  {
    std::sort( wires.begin( ), wires.end( ), [ ]( const auto& lhs, const auto& rhs )
    {
      return lhs.adc > rhs.adc;
    });

    uint16_t highest_adc = wires.front().adc;
    for (const auto &h : wires) {
      if (h.adc != highest_adc)
        break;
      used_readouts++;
      xmass += mappings.x(h.bus, h.channel) * h.adc;
      zmass += mappings.z(h.bus, h.channel) * h.adc;
      xsum += h.adc;
      zsum += h.adc;
      ret++;
    }
  }

  if (!grids.empty())
  {
    std::sort( grids.begin( ), grids.end( ), [ ]( const auto& lhs, const auto& rhs )
    {
      return lhs.adc > rhs.adc;
    });

    uint16_t highest_adc = grids.front().adc;
    for (const auto &h : grids) {
      if (h.adc != highest_adc)
        break;
      used_readouts++;
      ymass += mappings.y(h.bus, h.channel) * h.adc;
      ysum += h.adc;
      ret++;
    }
  }

  return ret;
}

void EfuPrioritized::reset() {
  xmass = 0;
  ymass = 0;
  zmass = 0;

  xsum = 0;
  ysum = 0;
  zsum = 0;

  time_ = 0;

  used_readouts = 0;
}

bool EfuPrioritized::event_good() const {
  return xsum && ysum && zsum;
}

}