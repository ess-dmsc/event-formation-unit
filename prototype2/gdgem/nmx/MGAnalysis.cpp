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

void EventAnalyzer::weighted(bool w) {
  weighted_ = w;
}

bool EventAnalyzer::weighted() const {
  return weighted_;
}


void EventAnalyzer::max_wire(uint16_t w) {
  max_wire_ = w;
}

void EventAnalyzer::max_z(uint16_t w) {
  max_z_ = w;
}

void EventAnalyzer::flipped_x(bool f) {
  flipped_x_ = f;
}

void EventAnalyzer::flipped_z(bool f) {
  flipped_z_ = f;
}


bool EventAnalyzer::flipped_x() const {
  return flipped_x_;
}

bool EventAnalyzer::flipped_z() const {
  return flipped_z_;
}

uint32_t EventAnalyzer::max_x() const {
  return max_wire() / max_z();
}

uint32_t EventAnalyzer::max_y() const {
  return max_grid();
}

uint16_t EventAnalyzer::max_z() const {
  return max_z_;
}

uint16_t EventAnalyzer::max_wire() const {
  return max_wire_;
}

uint16_t EventAnalyzer::max_grid() const {
  return max_grid_;
}

uint32_t EventAnalyzer::x_from_wire(uint16_t w) const {
  uint32_t ret = w / max_z();
  return flipped_x() ? (max_x() - 1u - ret) : ret;
}

uint32_t EventAnalyzer::y_from_grid(uint16_t g) const {
  return g;
}

uint32_t EventAnalyzer::z_from_wire(uint16_t w) const {
  uint32_t ret = w % max_z();
  return flipped_z() ? (max_z() - 1u - ret) : ret;
}

std::string EventAnalyzer::debug() const {
  std::string ret;

  ret += "MG analysis\n";

  ret += fmt::format("  size [{},{},{}]\n",
                     max_x(), max_y(), max_z());

  ret += fmt::format("  weighted = {}\n", (weighted_ ? "YES" : "no"));

  if (flipped_x_)
    ret += "  (flipped in X)\n";
  if (flipped_z_)
    ret += "  (flipped in Z)\n";

  return ret;
}

MultiDimResult EventAnalyzer::analyze(Event& event) const {
  MultiDimResult ret;

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
                return c1.weight > c2.weight;
              });

    uint16_t highest_adc = event.c1.hits.front().weight;
    for (const auto &h : event.c1.hits) {
      if (h.weight != highest_adc)
        break;
      stats_used_hits++;
      if (weighted_) {
        xmass += x_from_wire(h.coordinate) * h.weight;
        zmass += z_from_wire(h.coordinate) * h.weight;
        xsum += h.weight;
        zsum += h.weight;
      } else {
        xmass += x_from_wire(h.coordinate);
        zmass += z_from_wire(h.coordinate);
        xsum++;
        zsum++;
      }
    }

    ret.x.center = xmass / xsum;
    ret.z.center = zmass / zsum;
  }

  if (!event.c2.empty()) {

    double ymass{0};
    double ysum{0};

    std::sort(event.c2.hits.begin(), event.c2.hits.end(),
              [](const Hit &c1, const Hit &c2) {
                return c1.weight > c2.weight;
              });

    uint16_t highest_adc = event.c2.hits.front().weight;
    for (const auto &h : event.c2.hits) {
      if (h.weight != highest_adc)
        break;
      stats_used_hits++;
      if (weighted_) {
        ymass += y_from_grid(h.coordinate) * h.weight;
        ysum += h.weight;
      } else {
        ymass += y_from_grid(h.coordinate);
        ysum++;
      }
    }

    ret.y.center = ymass / ysum;
  }

  ret.time = event.time_start();
  ret.good =
      std::isfinite(ret.x.center) && (ret.x.center >= 0) &&
          std::isfinite(ret.y.center) && (ret.y.center >= 0) &&
          std::isfinite(ret.z.center) && (ret.z.center >= 0);
  return ret;
}

}
