/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Cluster.h>
#include <cmath>
#include <set>
#include <sstream>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Gem {

uint32_t utpcResults::utpc_center_rounded() const {
  return static_cast<uint32_t>(std::round(utpc_center));
}

std::string utpcResults::debug() const {
  std::stringstream ss;
  ss << "Utpc_center=" << utpc_center << " +-" << uncert_lower << " (+-" << uncert_upper << ") ";
  return ss.str();
}


utpcAnalyzer::utpcAnalyzer(bool weighted, uint16_t max_timebins, uint16_t max_timedif)
: weighted_(weighted)
, max_timebins_(max_timebins)
, max_timedif_(max_timedif)
{}

/// \todo make work with doubles (or not, if we decimate timestamps?)
utpcResults utpcAnalyzer::analyze(Cluster& cluster) const {
  utpcResults ret;

  if (cluster.hits.empty()) {
    return ret;
  }

  std::sort(cluster.hits.begin(), cluster.hits.end(),
      [](const Hit &c1, const Hit &c2) {
    return c1.time < c2.time;
  });

  double center_sum{0};
  double center_count{0};
  int16_t lspan_min = std::numeric_limits<int16_t>::max();
  int16_t lspan_max = std::numeric_limits<int16_t>::min();
  int16_t uspan_min = std::numeric_limits<int16_t>::max();
  int16_t uspan_max = std::numeric_limits<int16_t>::min();
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
      lspan_min = std::min(lspan_min, static_cast<int16_t>(e.coordinate));
      lspan_max = std::max(lspan_max, static_cast<int16_t>(e.coordinate));
    }
    if ((e.time >= earliest) && ((max_timebins_ > timebins.size()) || (timebins.count(e.time)))) {
      timebins.insert(e.time);
      uspan_min = std::min(uspan_min, static_cast<int16_t>(e.coordinate));
      uspan_max = std::max(uspan_max, static_cast<int16_t>(e.coordinate));
    } else {
      break;
    }
  }

  XTRACE(PROCESS, DEB, "center_sum=%f center_count=%f", center_sum,
         center_count);

  ret.utpc_center = center_sum / center_count;
  ret.uncert_lower = lspan_max - lspan_min + 1;
  ret.uncert_upper = uspan_max - uspan_min + 1;
  return ret;
}

uint64_t utpcAnalyzer::utpc_time(const Cluster& x, const Cluster& y) {
  return std::max(x.time_end(), y.time_end());
}

bool utpcAnalyzer::meets_lower_criterion(const utpcResults& x, const utpcResults& y,
                                  int16_t max_lu) {
  return (x.uncert_lower < max_lu) && (y.uncert_lower < max_lu);
}



}
