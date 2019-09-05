/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file MGAnalyzer.h
/// \brief MGAnalyzer class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/analysis/MgAnalyzer.h>
#include <common/reduction/ReducedEvent.h>
#include <common/reduction/Event.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

void MGAnalyzer::weighted(bool weighted) {
  weighted_ = weighted;
}

void MGAnalyzer::set_geometry(const Multigrid::ModuleGeometry &geom) {
  geometry_ = geom;
}

Multigrid::ModuleGeometry MGAnalyzer::geometry() const {
  return geometry_;
}

std::string MGAnalyzer::debug(const std::string &prepend) const {
  std::stringstream ss;
  ss << "MG analysis\n";
  ss << prepend << fmt::format("  weighted = {}\n", (weighted_ ? "YES" : "no"));
  ss << prepend << geometry_.debug(prepend + "  ");
  return ss.str();
}

ReducedEvent MGAnalyzer::analyze(Event &event) const {
  ReducedEvent ret;

  if (event.empty()) {
    return ret;
  }

  analyze_wires(WireCluster(event), ret.x, ret.z);

  ret.y = analyze_grids(GridCluster(event));

  ret.time = event.time_start();
  ret.good = ret.x.is_center_good() && ret.y.is_center_good() && ret.z.is_center_good();
  return ret;
}

ReducedHit MGAnalyzer::analyze_grids(Cluster &cluster) const {
  ReducedHit ret;
  ret.time = cluster.time_start();

  if (cluster.empty()) {
    return ret;
  }

  sort_by_decreasing_weight(cluster.hits);

  double ymass{0};
  double ysum{0};
  uint16_t highest_adc = cluster.hits.front().weight;
  for (const auto &h : cluster.hits) {
    if (h.weight != highest_adc)
      break;
    ret.hits_used++;
    stats_used_hits++;
    if (weighted_) {
      ymass += geometry_.y_from_grid(h.coordinate) * h.weight;
      ysum += h.weight;
    } else {
      ymass += geometry_.y_from_grid(h.coordinate);
      ysum++;
    }
  }
  ret.center = ymass / ysum;

  return ret;
}

void MGAnalyzer::analyze_wires(Cluster &cluster, ReducedHit &x, ReducedHit &z) const {

  x.time = z.time = cluster.time_start();

  if (cluster.empty()) {
    return;
  }

  sort_by_decreasing_weight(cluster.hits);

  double xmass{0};
  double zmass{0};
  double xsum{0};
  double zsum{0};

  uint16_t highest_adc = cluster.hits.front().weight;
  for (const auto &h : cluster.hits) {
    if (h.weight != highest_adc)
      break;
    x.hits_used++;
    z.hits_used++;
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

  x.center = xmass / xsum;
  z.center = zmass / zsum;
}

