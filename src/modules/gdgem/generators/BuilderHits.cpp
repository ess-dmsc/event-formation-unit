// Copyright (C) 2016, 2017 European Spallation Source ERIC
#include <common/time/TimeString.h>
#include <cstring>
#include <gdgem/generators/BuilderHits.h>

#include <common/debug/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/debug/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

namespace Gem {

BuilderHits::BuilderHits() { converted_data.reserve(9000 / sizeof(Hit)); }

void BuilderHits::process_buffer(char *buf, size_t size) {
  size_t count = std::min(size / sizeof(Hit), size_t(9000 / sizeof(Hit)));

  converted_data.resize(count);
  memcpy(converted_data.data(), buf, count * sizeof(Hit));

  // alloc approx 2x to reduce noise
  hit_buffer_x.reserve (count);
  hit_buffer_y.reserve (count);

  for (auto &hit : converted_data) {

    if (hit.weight == 0) {
      //XTRACE(BUILDER, DEB,
      //    "Accepted readout with adc=0, may distort uTPC results,
      //    hit={}", hit.to_string());
      // \todo What to do? Cannot be 0 for CoM in uTPC. Reject?
      hit.weight = 1;
    }

    if (hit.plane == 1) {
      XTRACE(BUILDER, DEB, "adding hit %s to plane y", hit.to_string().c_str());
      hit_buffer_y.emplace_back(hit);
    }

    if (hit.plane == 0) {
      XTRACE(BUILDER, DEB, "adding hit %s to plane x", hit.to_string().c_str());
      hit_buffer_x.emplace_back(hit);
    }
  }
}
} // namespace Gem
