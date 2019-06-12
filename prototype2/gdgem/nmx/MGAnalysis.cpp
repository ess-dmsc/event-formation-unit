/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/MGAnalysis.h>
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

namespace Gem {

void MGAnalyzer::weighted(bool w) {
  weighted_ = w;
}

bool MGAnalyzer::weighted() const {
  return weighted_;
}

std::string MGAnalyzer::debug() const {
  std::string ret;

  ret += "MG analysis\n";

  ret += fmt::format("  weighted = {}\n", (weighted_ ? "YES" : "no"));

  ret += geometry_.debug("  ");

  return ret;
}

MultiDimResult MGAnalyzer::analyze(Event& event) const {
  MultiDimResult ret;

  if (event.empty()) {
    return ret;
  }

  if (!event.c1.empty()) {

    double ymass{0};
    double ysum{0};

    std::sort(event.c1.hits.begin(), event.c1.hits.end(),
              [](const Hit &c2, const Hit &c1) {
                return c2.weight > c1.weight;
              });

    uint16_t highest_adc = event.c1.hits.front().weight;
    for (const auto &h : event.c1.hits) {
      if (h.weight != highest_adc)
        break;
      stats_used_hits++;
      if (weighted_) {
        ymass += geometry_.y_from_grid(h.coordinate) * h.weight;
        ysum += h.weight;
      } else {
        ymass += geometry_.y_from_grid(h.coordinate);
        ysum++;
      }
    }

    ret.y.center = ymass / ysum;
  }

  if (!event.c2.empty()) {
    double xmass{0};
    double zmass{0};
    double xsum{0};
    double zsum{0};

    std::sort(event.c2.hits.begin(), event.c2.hits.end(),
              [](const Hit &c2, const Hit &c1) {
                return c2.weight > c1.weight;
              });

    uint16_t highest_adc = event.c2.hits.front().weight;
    for (const auto &h : event.c2.hits) {
      if (h.weight != highest_adc)
        break;
      stats_used_hits++;
      if (weighted_) {
        xmass += geometry_.x_from_wire(h.coordinate) * h.weight;
        zmass += geometry_.z_from_wire(h.coordinate) * h.weight;
        xsum += h.weight;
        zsum += h.weight;
      } else {
        xmass += geometry_.x_from_wire(h.coordinate);
        zmass += geometry_.z_from_wire(h.coordinate);
        xsum++;
        zsum++;
      }
    }

    ret.x.center = xmass / xsum;
    ret.z.center = zmass / zsum;
  }

  ret.time = event.time_start();
  ret.good =
      std::isfinite(ret.x.center) && (ret.x.center >= 0) &&
          std::isfinite(ret.y.center) && (ret.y.center >= 0) &&
          std::isfinite(ret.z.center) && (ret.z.center >= 0);
  return ret;
}

}
