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
    HitsX.push_back(hit);
  } else if (hit.plane == PlaneY) {
    HitsY.push_back(hit);
  } else {
    XTRACE(CLUSTER, WAR, "bad plane %s", hit.to_string().c_str());
  }
}

void EventBuilder2D::flush() {
  matcher.matched_events.clear();

  sort_chronologically(HitsX);
  ClustererX.cluster(HitsX);
  // Clusterer flushes when time gap between hits is large enough, no need to force it between packets
  // ClustererX.flush();

  sort_chronologically(HitsY);
  ClustererY.cluster(HitsY);
  // ClustererY.flush();

  matcher.insert(PlaneX, ClustererX.clusters);
  matcher.insert(PlaneY, ClustererY.clusters);
  matcher.match(false);

  auto &e = matcher.matched_events;
  Events.insert(Events.end(), e.begin(), e.end());

  clear();
}

void EventBuilder2D::clear() {
  HitsX.mostly_clear(20);
  HitsY.mostly_clear(20);
}

