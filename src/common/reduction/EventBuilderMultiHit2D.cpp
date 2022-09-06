// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time-boxed event builder for 2D detector
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/EventBuilderMultiHit2D.h>
#include <fmt/format.h>

#include <algorithm>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF


EventBuilderMultiHit2D::EventBuilderMultiHit2D() { matcher.set_minimum_time_gap(timegap); }

void EventBuilderMultiHit2D::insert(Hit hit) {
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

void EventBuilderMultiHit2D::flush(bool full_flush) {
  XTRACE(CLUSTER, DEB, "flushing event builder");
  matcher.matched_events.clear();

  sort_chronologically(HitsX);
  ClustererX.cluster(HitsX);

  sort_chronologically(HitsY);
  ClustererY.cluster(HitsY);

  if(full_flush){
    flushClusterers();
  }

  matcher.insert(PlaneX, ClustererX.clusters);
  matcher.insert(PlaneY, ClustererY.clusters);
  matcher.match(full_flush);

  auto &e = matcher.matched_events;
  Events.insert(Events.end(), e.begin(), e.end());

  clearHits();
}

void EventBuilderMultiHit2D::clearHits() {
  HitsX.clear();
  HitsY.clear();
}

void EventBuilderMultiHit2D::flushClusterers() {
  ClustererX.flush();
  ClustererY.flush();
}



