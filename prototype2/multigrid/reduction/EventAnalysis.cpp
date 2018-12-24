/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/reduction/EventAnalysis.h>
#include <cmath>
#include <set>
#include <sstream>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

namespace Multigrid {

std::string NeutronPosition::debug() const {
  return fmt::format("x={}, y={}, z={}, t={}", x, y, z, time);
}


mgAnalyzer::mgAnalyzer(bool weighted)
: weighted_(weighted)
{}


NeutronPosition mgAnalyzer::analyze(Event& event) const {
  NeutronPosition ret;

  if (event.empty()) {
    return ret;
  }

  if (!event.c1.empty())
  {
    uint64_t xmass {0};
    uint64_t zmass {0};
    uint64_t xsum {0};
    uint64_t zsum {0};

    std::sort(event.c1.hits.begin(), event.c1.hits.end(),
              [](const Hit &c1, const Hit &c2) {
                return c1.weight < c2.weight;
              });

    uint16_t highest_adc = event.c1.hits.front().weight;
    for (const auto &h : event.c1.hits) {
      if (h.weight != highest_adc)
        break;
      //used_readouts++;
      if (weighted_) {
        xmass += mappings.x(h.bus, h.channel) * h.weight;
        zmass += mappings.z(h.bus, h.channel) * h.weight;
        xsum += h.weight;
        zsum += h.weight;
      }
      else {
        xmass += mappings.x(h.bus, h.channel);
        zmass += mappings.z(h.bus, h.channel);
        xsum++;
        zsum++;
      }
    }

    ret.x = xmass / xsum;
    ret.z = zmass / zsum;
  }

  if (!event.c2.empty()) {

    uint64_t ymass {0};
    uint64_t ysum {0};

    std::sort(event.c2.hits.begin(), event.c2.hits.end(),
              [](const Hit &c1, const Hit &c2) {
                return c1.weight < c2.weight;
              });

    uint16_t highest_adc = event.c2.hits.front().weight;
    for (const auto &h : event.c2.hits) {
      if (h.weight != highest_adc)
        break;
      //used_readouts++;
      if (weighted_) {
        ymass += mappings.y(h.bus, h.channel) * h.weight;
        ysum += h.weight;
      } else {
        ymass += mappings.y(h.bus, h.channel);
        ysum++;
      }
    }

    ret.y = ymass / ysum;
  }

  ret.time = event.time_start();
  ret.good = std::isfinite(ret.x) && std::isfinite(ret.y) && std::isfinite(ret.z);
  return ret;
}



}
