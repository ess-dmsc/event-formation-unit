/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/BuilderHits.h>
#include <common/TimeString.h>
#include <cstring>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

namespace Gem {

BuilderHits::BuilderHits() {
  converted_data.reserve(9000 / sizeof(Hit));
}

void BuilderHits::process_buffer(char *buf, size_t size) {
  size_t count = std::min(size / sizeof(Hit), size_t(9000 / sizeof(Hit)));

  converted_data.resize(count);
  memcpy(converted_data.data(), buf, count * sizeof(Hit));

  for (auto &hit : converted_data) {

    if (hit.weight == 0) {
//        LOG(PROCESS, Sev::Warning,
//            "Accepted readout with adc=0, may distort uTPC results, hit={}",
//            hit.debug());
      // \todo What to do? Cannot be 0 for CoM in uTPC. Reject?
      hit.weight = 1;
    }


    if (hit.plane == 1) {
      hit_buffer_y.emplace_back(hit);
    }

    if (hit.plane == 0) {
      hit_buffer_x.emplace_back(hit);
    }
  }
}
}
