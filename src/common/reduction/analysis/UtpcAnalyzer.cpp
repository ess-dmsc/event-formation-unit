/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file utpcAnalyzer.h
/// \brief utpcAnalyzer class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/analysis/UtpcAnalyzer.h>
#include <cmath>
#include <set>

#include <common/debug/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/debug/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

utpcAnalyzer::utpcAnalyzer(bool weighted, uint16_t max_timebins, uint16_t max_timedif)
    : weighted_(weighted), max_timebins_(max_timebins), max_timedif_(max_timedif) {}

ReducedHit utpcAnalyzer::analyze(Cluster &cluster) const {
  ReducedHit ret;

  if (cluster.hits.empty()) {
    return ret;
  }

  sort_chronologically(cluster.hits);

  double center_sum{0};
  double center_count{0};
  uint16_t lspan_min = std::numeric_limits<uint16_t>::max();
  uint16_t lspan_max = std::numeric_limits<uint16_t>::min();
  uint16_t uspan_min = std::numeric_limits<uint16_t>::max();
  uint16_t uspan_max = std::numeric_limits<uint16_t>::min();
  uint64_t earliest = std::min(cluster.time_start(), cluster.time_end()
      - static_cast<uint64_t>(max_timedif_));
  std::set<uint64_t> timebins;
  for (auto it = cluster.hits.rbegin(); it != cluster.hits.rend(); ++it) {
    auto e = *it;
    if (e.time == cluster.time_end()) {
      if (weighted_) {
        center_sum += (e.coordinate * e.weight);
        center_count += e.weight;
      } else {
        center_sum += e.coordinate;
        center_count++;
      }
      ret.hits_used++;
      stats_used_hits++;
      lspan_min = std::min(lspan_min, e.coordinate);
      lspan_max = std::max(lspan_max, e.coordinate);
    }
    if ((e.time >= earliest) && ((max_timebins_ > timebins.size()) || (timebins.count(e.time)))) {
      timebins.insert(e.time);
      uspan_min = std::min(uspan_min, e.coordinate);
      uspan_max = std::max(uspan_max, e.coordinate);
    } else {
      break;
    }
  }

  LOG(PROCESS, Sev::Debug, "uTPC center_sum={} center_count={}",
      center_sum,
      center_count);

  ret.center = center_sum / center_count;
  ret.uncert_lower = lspan_max - lspan_min + 1;
  ret.uncert_upper = uspan_max - uspan_min + 1;
  return ret;
}

ReducedEvent utpcAnalyzer::analyze(Event &event) const {
  ReducedEvent ret;
  ret.x = analyze(event.ClusterA);
  ret.y = analyze(event.ClusterB);
  ret.good = std::isfinite(ret.x.center) && std::isfinite(ret.y.center);
  ret.time = utpc_time(event);
  return ret;
}

uint64_t utpcAnalyzer::utpc_time(const Event &e) {
  // \todo is this what we want?
  return std::max(e.ClusterA.time_end(), e.ClusterB.time_end());
}

bool utpcAnalyzer::meets_lower_criterion(const ReducedHit &x, const ReducedHit &y,
                                         int16_t max_lu) {
  return (x.uncert_lower < max_lu) && (y.uncert_lower < max_lu);
}

// GCOVR_EXCL_START
std::string utpcAnalyzer::debug(const std::string& prepend) const {
  std::stringstream ss;
  ss << "uTPC analysis\n";
  ss << prepend << fmt::format("  weighted = {}\n", (weighted_ ? "YES" : "no"));
  ss << prepend << fmt::format("  max_timebins = {}\n", max_timebins_);
  ss << prepend << fmt::format("  max_timedif = {}\n", max_timedif_);
  return ss.str();
}
// GCOVR_EXCL_STOP
