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

void mgAnalyzer::weighted(bool w) {
  weighted_ = w;
}

bool mgAnalyzer::weighted() const {
  return weighted_;
}


NeutronPosition mgAnalyzer::analyze(Event &event) const {
  NeutronPosition ret;

  if (event.empty()) {
    return ret;
  }

  if (!event.c1.empty()) {
    double xmass{0};
    double zmass{0};
    double xsum{0};
    double zsum{0};

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
        xmass += mappings.x_from_wire(h.coordinate) * h.weight;
        zmass += mappings.z_from_wire(h.coordinate) * h.weight;
        xsum += h.weight;
        zsum += h.weight;
      } else {
        xmass += mappings.x_from_wire(h.coordinate);
        zmass += mappings.z_from_wire(h.coordinate);
        xsum++;
        zsum++;
      }
    }

    ret.x = xmass / xsum;
    ret.z = zmass / zsum;
  }

  if (!event.c2.empty()) {

    double ymass{0};
    double ysum{0};

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
        ymass += mappings.y_from_grid(h.coordinate) * h.weight;
        ysum += h.weight;
      } else {
        ymass += mappings.y_from_grid(h.coordinate);
        ysum++;
      }
    }

    ret.y = ymass / ysum;
  }

  ret.time = event.time_start();
  ret.good =
      std::isfinite(ret.x) && (ret.x >= 0) &&
          std::isfinite(ret.y) && (ret.y >= 0) &&
          std::isfinite(ret.z) && (ret.z >= 0);
  return ret;
}

}
