/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/MgAnalyzer.h>
#include <common/clustering/AbstractClusterer.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

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

ReducedEvent MGAnalyzer::analyze(Event& event) const {
  ReducedEvent ret;

  if (event.empty()) {
    return ret;
  }

  // grid
  if (!event.cluster1.empty()) {

    double ymass{0};
    double ysum{0};

    sort_by_decreasing_weight(event.cluster1.hits);

    uint16_t highest_adc = event.cluster1.hits.front().weight;
    for (const auto &h : event.cluster1.hits) {
      if (h.weight != highest_adc)
        break;
      ret.y.hits_used++;
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

  // wire
  if (!event.cluster2.empty()) {
    double xmass{0};
    double zmass{0};
    double xsum{0};
    double zsum{0};

    sort_by_decreasing_weight(event.cluster2.hits);

    uint16_t highest_adc = event.cluster2.hits.front().weight;
    for (const auto &h : event.cluster2.hits) {
      if (h.weight != highest_adc)
        break;
      ret.x.hits_used++;
      ret.z.hits_used++;
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
  ret.good = ret.x.is_center_good() && ret.y.is_center_good() && ret.z.is_center_good();
  ret.module = event.cluster1.plane() / 2;
  return ret;
}
