// Copyright (C) 2016, 2017 European Spallation Source ERIC

#include <algorithm>
#include <cmath>
#include <common/reduction/analysis/EventAnalyzer.h>
#include <set>
#include <sstream>

#include <common/debug/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/debug/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

EventAnalyzer::EventAnalyzer(const std::string &time_algorithm)
    : time_algorithm_(time_algorithm) {
  if (time_algorithm_ == "center-of-mass") {
    time_algo = TA_center_of_mass;
  } else if (time_algorithm_ == "charge2") {
    time_algo = TA_charge2;
  } else {
    if (time_algorithm_ == "utpc") {
      time_algo = TA_utpc;
    } else {
      time_algo = TA_utpc_weighted;
    }
  }
}

ReducedHit EventAnalyzer::analyze(Cluster &cluster) const {
  ReducedHit ret;

  if (cluster.hits.empty()) {
    return ret;
  }

  if (time_algo == TA_center_of_mass) {
    ret.center = cluster.coordCenter();
    ret.time = cluster.timeCenter();
  } else if (time_algo == TA_charge2) {
    ret.center = cluster.coordCenter2();
    ret.time = cluster.timeCenter2();
  } else {
    if (time_algo == TA_utpc) {
      ret.center = cluster.coordUtpc(false);
    } else {
      ret.center = cluster.coordUtpc(true);
    }
    ret.time = cluster.timeEnd(); /// \TODO we get he nicest results for
                                  /// HitGenerator with 'cluster.timeStart()'.
  }

  return ret;
}

ReducedEvent EventAnalyzer::analyze(Event &event) const {
  ReducedEvent ret;
  ret.x = analyze(event.ClusterA);
  ret.y = analyze(event.ClusterB);
  ret.good = std::isfinite(ret.x.center) && std::isfinite(ret.y.center);
  ret.time = std::min(ret.x.time, ret.y.time);
  return ret;
}

std::string EventAnalyzer::debug(const std::string &prepend) const {
  std::stringstream ss;
  ss << "Event analysis\n";
  ss << prepend << fmt::format("  time_algorithm = {}\n", time_algorithm_);
  return ss.str();
}