/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Event.h>
#include <cmath>
#include <sstream>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

void Event::insert_eventlet(const Eventlet &e) {
  if (e.plane_id == 1) { /**< @todo deal with multiple panels */
    y.insert_eventlet(e);
  } else if (e.plane_id == 0) {
    x.insert_eventlet(e);
  }
}

void Event::merge(Cluster& cluster, uint8_t plane_id)
{
  if (plane_id == 1) { /**< @todo deal with multiple panels */
    y.merge(cluster);
  } else if (plane_id == 0) {
    x.merge(cluster);
  }
}

void Event::analyze(bool weighted, int16_t max_timebins,
                       int16_t max_timedif) {
  XTRACE(PROCESS, DEB, "x.entries.size(): %lu, y.entries.size(): %lu\n",
         x.entries.size(), y.entries.size());
  if (x.entries.size()) {
    x.analyze(weighted, max_timebins, max_timedif);
  }
  if (y.entries.size()) {
    y.analyze(weighted, max_timebins, max_timedif);
  }
  valid_ = x.entries.size() && y.entries.size();
  if (valid_) {
    time_start_ = std::min(x.time_start, y.time_start);
  }
}

bool Event::time_overlap_thresh(const Cluster& other, double thresh) const
{
  auto ovr = time_overlap(other);
  return (((ovr / other.time_span()) + (ovr / time_span())) > thresh);
}

double Event::time_overlap(const Cluster& other) const
{
  auto latest_start = std::max(other.time_start, time_start());
  auto earliest_end = std::min(other.time_end, time_end());
  if (latest_start > earliest_end)
    return 0;
  return (earliest_end - latest_start);
}

double Event::time_end() const
{
  return std::max(x.time_end, y.time_end);
}

double Event::time_start() const
{
  return std::min(x.time_start, y.time_start);
}

double Event::time_span() const
{
  return (time_end() - time_start());
}

bool Event::valid() const { return valid_; }

bool Event::empty() const
{
  return x.entries.empty() && y.entries.empty();
}

bool Event::meets_lower_cirterion(int16_t max_lu) const {
  return (x.uncert_lower < max_lu) && (y.uncert_lower < max_lu);
}

double Event::utpc_earliest() const
{
  return time_start_;
}

std::string Event::debug() const {
  std::stringstream ss;
  ss << "Tstart=" << time_start_;
  if (valid_)
    ss << "  GOOD\n";
  else
    ss << "  BAD\n";
  ss << "  X:\n" << x.debug();
  ss << "  Y:\n" << y.debug();
  return ss.str();
}

void Event::debug2() {
  if (x.entries.size()) {
    printf("x strips: ");
    for (auto xstrips : x.entries) {
      printf("%d ", xstrips.strip);
    }
    printf("\n");
  }
  if (y.entries.size()) {
    printf("y strips: ");
    for (auto ystrips : y.entries) {
      printf("%d ", ystrips.strip);
    }
    printf("\n");
  }
}
