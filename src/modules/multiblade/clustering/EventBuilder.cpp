// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time-boxed event builder for Multi-Blade
///
//===----------------------------------------------------------------------===//

#include <multiblade/clustering/EventBuilder.h>
#include <algorithm>
#include <fmt/format.h>

#include <common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

EventBuilder::EventBuilder() {
  matcher.set_minimum_time_gap(timegap);
}


EventBuilder::EventBuilder(uint32_t BoxSize) : TimeBoxSize(BoxSize) {
  matcher.set_minimum_time_gap(timegap);
}

void EventBuilder::insert(Hit hit) {
  if ((hit.time - TimeBoxT0) >= TimeBoxSize) {
    flush();
    TimeBoxT0 = hit.time;
    XTRACE(CLUSTER, DEB, "NEW TIME BOX ===================================");
  }

  XTRACE(CLUSTER, DEB, "hit: {%u, %llu %u %u}", hit.plane, hit.time, hit.coordinate, hit.weight);

  if (hit.plane == WirePlane) {
    p0.push_back(hit);
  }
  else if (hit.plane == StripPlane) {
    p1.push_back(hit);
  }
  else {
    XTRACE(CLUSTER, WAR, "bad plane %s", hit.to_string().c_str());
  }
}

void EventBuilder::flush() {
  matcher.matched_events.clear();

  sort_chronologically(p0);
  c0.cluster(p0);
  c0.flush();

  sort_chronologically(p1);
  c1.cluster(p1);
  c1.flush();

  matcher.insert(WirePlane, c0.clusters);
  matcher.insert(StripPlane, c1.clusters);
  matcher.match(true);

  auto & e = matcher.matched_events;
  Events.insert(Events.end(), e.begin(), e.end());

  clear();
}

void EventBuilder::clear() {
  p0.clear();
  p1.clear();
}


}
