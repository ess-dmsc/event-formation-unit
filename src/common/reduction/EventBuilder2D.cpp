// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time-boxed event builder for Multi-Blade
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/EventBuilder2D.h>
#include <fmt/format.h>

#include <algorithm>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF

EventBuilder2D::EventBuilder2D() { matcher.set_minimum_time_gap(timegap); }

void EventBuilder2D::insert(Hit hit) {
  XTRACE(CLUSTER, DEB, "hit: {%u, %llu %u %u}", hit.plane, hit.time,
         hit.coordinate, hit.weight);

  if (hit.plane == PlaneX) {
    XTRACE(CLUSTER, DEB, "pushing hit to HitsX");
    HitsX.push_back(hit);
  } else if (hit.plane == PlaneY) {
    XTRACE(CLUSTER, DEB, "pushing hit to HitsY");
    HitsY.push_back(hit);
  } else {
    XTRACE(CLUSTER, WAR, "bad plane %s", hit.to_string().c_str());
  }
}

void EventBuilder2D::flush(bool full_flush) {
  XTRACE(CLUSTER, DEB, "flushing event builder");
  matcher.matched_events.clear();

  sort_chronologically(HitsX);
  ClustererX.cluster(HitsX);

  sort_chronologically(HitsY);
  ClustererY.cluster(HitsY);

  if (full_flush) {
    flushClusterers();
  }

  matcher.insert(PlaneX, ClustererX.clusters);
  matcher.insert(PlaneY, ClustererY.clusters);
  matcher.match(full_flush);

  auto &e = matcher.matched_events;
  Events.insert(Events.end(), e.begin(), e.end());

  clearHits();
}

void EventBuilder2D::clearHits() {
  HitsX.clear();
  HitsY.clear();
}

void EventBuilder2D::flushClusterers() {
  ClustererX.flush();
  ClustererY.flush();
}
