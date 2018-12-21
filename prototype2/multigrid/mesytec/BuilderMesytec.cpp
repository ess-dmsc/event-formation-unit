/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/BuilderMesytec.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

void BuilderMesytec::parse(Buffer<uint8_t> buffer) {


  stats_discarded_bytes += sis3153parser.parse(Buffer<uint8_t>(buffer));

  for (const auto &b : sis3153parser.buffers) {

    stats_discarded_bytes += vmmr16Parser.parse(b);

    if (vmmr16Parser.converted_data.empty())
      continue;

    if (dumpfile) {
      dumpfile->push(vmmr16Parser.converted_data);
    }

    if (vmmr16Parser.externalTrigger()) {

    }

    stats_trigger_count = vmmr16Parser.trigger_count();

    for (const auto& r : vmmr16Parser.converted_data) {
      if (digital_geometry.isWire(r.bus, r.channel)) {
        hit.weight = digital_geometry.rescale(r.bus, r.channel, r.adc);
        if (!digital_geometry.is_valid(r.bus, r.channel, hit.weight)) {
          stats_readout_filter_rejects++;
          continue;
        }
        hit.coordinate = digital_geometry.wire(r.bus, r.channel);
        hit.plane = 0;
      }
      else if (digital_geometry.isGrid(r.bus, r.channel)) {
        hit.weight = digital_geometry.rescale(r.bus, r.channel, r.adc);
        if (!digital_geometry.is_valid(r.bus, r.channel, hit.weight)) {
          stats_readout_filter_rejects++;
          continue;
        }
        hit.coordinate = digital_geometry.grid(r.bus, r.channel);
        hit.plane = 1;
      }
      else if (r.external_trigger) {
        hit.plane = 99;
        hit.coordinate = 0;
        hit.weight = 0;
      }
      else {
        stats_digital_geom_errors++;
        continue;
      }

      hit.time = r.total_time;
      ConvertedData.push_back(hit);
    }

  }
}

}
