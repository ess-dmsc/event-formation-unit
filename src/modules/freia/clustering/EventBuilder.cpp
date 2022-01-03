// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time-boxed event builder for Multi-Blade
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <fmt/format.h>
#include <freia/clustering/EventBuilder.h>

#include <common/debug/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF

namespace Freia {

EventBuilder::EventBuilder() { matcher.set_minimum_time_gap(timegap); }

void EventBuilder::insert(Hit hit) {
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


void EventBuilder::flush() {
  matcher.matched_events.clear();

  sort_chronologically(HitsX);
  ClustersX.cluster(HitsX);
  ClustersX.flush();

  sort_chronologically(HitsY);
  ClustersY.cluster(HitsY);
  ClustersY.flush();

  matcher.insert(PlaneX, ClustersX.clusters);
  matcher.insert(PlaneY, ClustersY.clusters);
  matcher.match(true);

  auto &e = matcher.matched_events;
  Events.insert(Events.end(), e.begin(), e.end());

  clear();
}

void EventBuilder::clear() {
  HitsX.clear();
  HitsY.clear();
}

} // namespace Multiblade
