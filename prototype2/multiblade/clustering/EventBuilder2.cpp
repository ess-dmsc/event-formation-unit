/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multiblade/clustering/EventBuilder2.h>
#include <algorithm>
#include <fmt/format.h>

#include <common/Trace.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

void EventBuilder2::insert(Hit hit) {
  if (hit.plane == 0) {
    p0.push_back(hit);
  }
  else if (hit.plane == 1) {
    p1.push_back(hit);
  }
  else {
    XTRACE(DATA, WAR, "bad plane %s", hit.debug().c_str());
  }
}

void EventBuilder2::flush() {

  std::sort(p0.begin(), p0.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.coordinate < e2.coordinate;
            });
  c0.cluster(p0);
  c0.flush();

  std::sort(p1.begin(), p1.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.coordinate < e2.coordinate;
            });
  c1.cluster(p1);
  c1.flush();

  matcher.insert(0, c0.clusters);
  matcher.insert(1, c1.clusters);
  matcher.match(true);

  clear();
}

void EventBuilder2::clear() {
  p0.clear();
  p1.clear();
}


}
