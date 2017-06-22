/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/EventNMX.h>
#include <common/Trace.h>
#include <set>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

// #include <iostream>

void PlaneNMX::insert_eventlet(const Eventlet &e) {
  if (!e.adc)
    return;
  if (entries.empty()) {
    time_start = time_end = e.time;
  }
  entries.push_back(e);
  integral += e.adc;
  time_start = std::min(time_start, e.time);
  time_end = std::max(time_end, e.time);
}

void PlaneNMX::analyze(bool weighted, uint16_t max_timebins,
                       uint16_t max_timedif) {
  if (entries.empty())
    return;
  double center_sum{0};
  double center_count{0};
  int16_t lspan_min = std::numeric_limits<int16_t>::max();
  int16_t lspan_max = std::numeric_limits<int16_t>::min();
  int16_t uspan_min = std::numeric_limits<int16_t>::max();
  int16_t uspan_max = std::numeric_limits<int16_t>::min();
  uint64_t earliest =
      std::min(time_start, time_end - static_cast<uint64_t>(max_timedif));
  std::set<uint64_t> timebins;
  for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
    auto e = *it;
    if (e.time == time_end) {
      if (weighted) {
        center_sum += (e.strip * e.adc);
        center_count += e.adc;
      } else {
        center_sum += e.strip;
        center_count++;
      }
      lspan_min = std::min(lspan_min, static_cast<int16_t>(e.strip));
      lspan_max = std::max(lspan_max, static_cast<int16_t>(e.strip));
    }
    if ((e.time >= earliest) &&
        ((max_timebins > timebins.size()) || (timebins.count(e.time)))) {
      timebins.insert(e.time);
      uspan_min = std::min(uspan_min, static_cast<int16_t>(e.strip));
      uspan_max = std::max(uspan_max, static_cast<int16_t>(e.strip));
    } else
      break;
  }
  // std::cout << "center_sum=" << center_sum
  //           << " center_count=" << center_count << "\n";


  center = center_sum / center_count;
  uncert_lower = lspan_max - lspan_min + 1;
  uncert_upper = uspan_max - uspan_min + 1;
}

void EventNMX::insert_eventlet(const Eventlet &e) {
  if (e.plane_id == 1) { /**< @todo deal with multiple panels */
    y.insert_eventlet(e);
  } else if (e.plane_id == 0) {
    x.insert_eventlet(e);
  } else {
    printf("Invalid plane id\n");
  }
}

void EventNMX::analyze(bool weighted, int16_t max_timebins,
                       int16_t max_timedif) {
  XTRACE(PROCESS, DEB, "x.entries.size(): %lu, y.entries.size(): %lu\n",
         x.entries.size(), y.entries.size());
  if (x.entries.size()) {
    x.analyze(weighted, max_timebins, max_timedif);
  }
  if (y.entries.size()) {
    y.analyze(weighted, max_timebins, max_timedif);
  }
  good_ = x.entries.size() && y.entries.size();
  if (good_) {
    time_start_ = std::min(x.time_start, y.time_start);
  }
}

bool EventNMX::good() const { return good_; }

uint64_t EventNMX::time_start() const
{
  return time_start_;
}
