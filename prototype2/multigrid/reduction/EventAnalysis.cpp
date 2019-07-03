/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/reduction/EventAnalysis.h>
#include <common/clustering/AbstractClusterer.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

namespace Multigrid {

void EventAnalyzer::weighted(bool w) {
  weighted_ = w;
}

bool EventAnalyzer::weighted() const {
  return weighted_;
}

ReducedEvent EventAnalyzer::analyze(Event &event) {
  ReducedEvent ret;

  if (event.empty()) {
    return ret;
  }

  if (!event.cluster1.empty()) {
    double xmass{0};
    double zmass{0};
    double xsum{0};
    double zsum{0};

    sort_chronologically(event.cluster1.hits);

    uint16_t highest_adc = event.cluster1.hits.front().weight;
    for (const auto &h : event.cluster1.hits) {
      if (h.weight != highest_adc)
        break;
      stats_used_hits++;
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

    ret.x.center = xmass / xsum;
    ret.z.center = zmass / zsum;
  }

  if (!event.cluster2.empty()) {

    double ymass{0};
    double ysum{0};

    sort_chronologically(event.cluster2.hits);

    uint16_t highest_adc = event.cluster2.hits.front().weight;
    for (const auto &h : event.cluster2.hits) {
      if (h.weight != highest_adc)
        break;
      stats_used_hits++;
      if (weighted_) {
        ymass += mappings.y_from_grid(h.coordinate) * h.weight;
        ysum += h.weight;
      } else {
        ymass += mappings.y_from_grid(h.coordinate);
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
