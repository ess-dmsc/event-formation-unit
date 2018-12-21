/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/generators/BuilderReadouts.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

BuilderReadouts::BuilderReadouts() {
  converted_data.reserve(9000 / sizeof(Readout));
}

void BuilderReadouts::parse(Buffer<uint8_t> buffer) {

  size_t count = std::min(buffer.size / sizeof(Readout),
                          size_t(9000 / sizeof(Readout)));

  converted_data.resize(count);
  memcpy(converted_data.data(), buffer.address, count * sizeof(Readout));

  //stats_trigger_count = vmmr16Parser.trigger_count();

  for (const auto &r : converted_data) {
    if (r.external_trigger) {
      hit.plane = 99;
      hit.coordinate = 0;
      hit.weight = 0;
    } else if (digital_geometry.isWire(r.bus, r.channel)) {
      hit.weight = digital_geometry.rescale(r.bus, r.channel, r.adc);
      if (!digital_geometry.is_valid(r.bus, r.channel, hit.weight)) {
        stats_readout_filter_rejects++;
        continue;
      }
      hit.coordinate = digital_geometry.wire(r.bus, r.channel);
      hit.plane = 0;
    } else if (digital_geometry.isGrid(r.bus, r.channel)) {
      hit.weight = digital_geometry.rescale(r.bus, r.channel, r.adc);
      if (!digital_geometry.is_valid(r.bus, r.channel, hit.weight)) {
        stats_readout_filter_rejects++;
        continue;
      }
      hit.coordinate = digital_geometry.grid(r.bus, r.channel);
      hit.plane = 1;
    } else {
      stats_digital_geom_errors++;
      continue;
    }

    hit.time = r.total_time;
    ConvertedData.push_back(hit);
  }

}

}
