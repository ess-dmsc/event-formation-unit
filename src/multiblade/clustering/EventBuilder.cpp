/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multiblade/clustering/EventBuilder.h>
#include <algorithm>
#include <fmt/format.h>

#include <common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#error asdfasdfasdf

namespace Multiblade {

EventBuilder::EventBuilder() {
  matcher.set_minimum_time_gap(timegap);
}

void EventBuilder::insert(Hit hit) {
  if (hit.plane == 0) {
    p0.push_back(hit);
  }
  else if (hit.plane == 1) {
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

  matcher.insert(0, c0.clusters);
  matcher.insert(1, c1.clusters);
  matcher.match(true);

  clear();
}

void EventBuilder::clear() {
  p0.clear();
  p1.clear();
}


}
