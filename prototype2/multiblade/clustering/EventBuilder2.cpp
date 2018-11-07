/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multiblade/clustering/EventBuilder2.h>
#include <algorithm>
#include <fmt/format.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

void HitQueue::insert(Readout hit) {
  if (readouts.size() && (readouts.back().local_time > hit.local_time)) {
    XTRACE(DATA, DEB, "*** queue[%d-%d] time overflow: %s > %s",
           id, id + 3,
           readouts.back().debug().c_str(),
           hit.debug().c_str());
  }
  readouts.push_back(hit);
}

DigitizerQueue::DigitizerQueue() {
  groups.resize(8);
  for (uint16_t i = 0; i < 8; ++i)
    groups[i].id = i * 8;
}

void DigitizerQueue::insert(Readout hit) {
  size_t idx = hit.channel / uint16_t(8);
  auto &readouts = groups[idx].readouts;
  if (readouts.size() && (readouts.back().local_time > hit.local_time)) {
    XTRACE(DATA, DEB, "*** queue[%d:%d-%d] time overflow: %s > %s",
           id, 8 * idx, 8 * idx + 7,
           readouts.back().debug().c_str(),
           hit.debug().c_str());
  }
  readouts.push_back(hit);
}

}
