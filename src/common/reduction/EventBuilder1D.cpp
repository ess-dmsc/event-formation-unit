// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time-boxed event builder for Multi-Blade
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/EventBuilder1D.h>
#include <fmt/format.h>

#include <algorithm>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

EventBuilder1D::EventBuilder1D() {}

void EventBuilder1D::insert(Hit hit) {
  XTRACE(CLUSTER, DEB, "hit: {%u, %llu %u %u}", hit.plane, hit.time,
         hit.coordinate, hit.weight);

  if (hit.plane == Plane) {
    XTRACE(CLUSTER, DEB, "pushing hit to hits");
    Hits.push_back(hit);
  } else {
    XTRACE(CLUSTER, WAR, "bad plane %s", hit.to_string().c_str());
  }
}

void EventBuilder1D::flush(bool full_flush) {
  XTRACE(CLUSTER, DEB, "flushing event builder");
  
  sort_chronologically(Hits);
  Clusterer.cluster(Hits);

 
  auto &e = Clusterer.clusters;
  Events.insert(Events.end(), e.begin(), e.end());

  clearHits();
}

void EventBuilder1D::clearHits() {
  HitsX.clear();
  HitsY.clear();
}

void EventBuilder1D::flushClusterers() {
  ClustererX.flush();
  ClustererY.flush();
}
