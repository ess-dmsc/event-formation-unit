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

    stats_trigger_count = vmmr16Parser.trigger_count();

    if (dumpfile) {
      dumpfile->push(vmmr16Parser.converted_data);
    }

    for (const auto& r : vmmr16Parser.converted_data) {
      if (r.external_trigger) {
        hit.plane = external_trigger_plane;
        hit.coordinate = 0;
        hit.weight = 0;
      } else if (digital_geometry.isWire(r.bus, r.channel)) {
        hit.weight = digital_geometry.rescale(r.bus, r.channel, r.adc);
        if (!digital_geometry.is_valid(r.bus, r.channel, hit.weight)) {
          stats_readout_filter_rejects++;
          continue;
        }
        hit.coordinate = digital_geometry.wire(r.bus, r.channel);
        hit.plane = wire_plane;
      } else if (digital_geometry.isGrid(r.bus, r.channel)) {
        hit.weight = digital_geometry.rescale(r.bus, r.channel, r.adc);
        if (!digital_geometry.is_valid(r.bus, r.channel, hit.weight)) {
          stats_readout_filter_rejects++;
          continue;
        }
        hit.coordinate = digital_geometry.grid(r.bus, r.channel);
        hit.plane = grid_plane;
      } else {
        XTRACE(PROCESS, DEB, "Bad geometry %s", r.debug().c_str());
        stats_digital_geom_errors++;
        continue;
      }

      hit.time = r.total_time;
      ConvertedData.push_back(hit);
    }

  }
}

}
