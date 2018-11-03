/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/clustering/Event.h>
#include <cmath>
#include <sstream>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

void Event::insert_hit(const Hit &e) {
  if (e.plane == 1) { /**< \todo deal with multiple panels */
    y.insert_hit(e);
  } else if (e.plane == 0) {
    x.insert_hit(e);
  }
}

void Event::merge(Cluster &cluster) {
  if (cluster.plane() == 1) { /**< \todo deal with multiple panels */
    y.merge(cluster);
  } else if (cluster.plane() == 0) {
    x.merge(cluster);
  }
}

bool Event::empty() const {
  return x.empty() && y.empty();
}

uint64_t Event::time_end() const {
  if (x.empty())
    return y.time_end();
  if (y.empty())
    return x.time_end();
  return std::max(x.time_end(), y.time_end());
}

uint64_t Event::time_start() const {
  if (x.empty())
    return y.time_start();
  if (y.empty())
    return x.time_start();
  return std::min(x.time_start(), y.time_start());
}

uint64_t Event::time_span() const {
  return (time_end() - time_start());
}

uint64_t Event::time_overlap(const Cluster &other) const {
  auto latest_start = std::max(other.time_start(), time_start());
  auto earliest_end = std::min(other.time_end(), time_end());
  if (latest_start > earliest_end)
    return 0;
  return (earliest_end - latest_start);
}

bool Event::time_overlap_thresh(const Cluster &other, double thresh) const {
  double ovr = time_overlap(other);
  return (((ovr / other.time_span()) + (ovr / time_span())) > thresh);
}

std::string Event::debug() const {
  std::stringstream ss;
  ss << "  X:\n" << x.debug();
  ss << "  Y:\n" << y.debug();
  return ss.str();
}
