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

EventBuilder::EventBuilder(uint32_t BoxSize) : TimeBoxSize(BoxSize) {
  matcher.set_minimum_time_gap(timegap);
}

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


void EventBuilder::insertDone() {
  XTRACE(CLUSTER, DEB, "HitsX %u, HitsY %u", HitsX.size(), HitsY.size());
  uint32_t TotReadouts = HitsX.size() + HitsY.size();
  bool OneIsEmpty = HitsX.size() and HitsX.size();

  if ((TotReadouts < 500) and (TotReadouts != 0)) {
    XTRACE(CLUSTER, INF, "Too few readouts (%u) keep buffering", TotReadouts);
    return; // More Hits are needed
  }
  if (OneIsEmpty) {
    XTRACE(CLUSTER, INF, "Either X or Y has no readouts");
    return; // More Hits are needed
  }

  XTRACE(CLUSTER, INF, "HitsX %u, HitsY %u", HitsX.size(), HitsY.size());
  flush();
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
