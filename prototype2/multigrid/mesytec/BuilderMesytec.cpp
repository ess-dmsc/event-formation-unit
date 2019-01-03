/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/BuilderMesytec.h>
#include <common/TimeString.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

BuilderMesytec::BuilderMesytec(const SequoiaGeometry &geometry, bool spoof_time,
                               std::string dump_dir)
    : digital_geometry_(geometry) {
  vmmr16Parser_.spoof_high_time(spoof_time);
  if (!dump_dir.empty()) {
    dumpfile_ = ReadoutFile::create(dump_dir + "mgmesytec_readouts_" + timeString(), 100);
  }

}

std::string BuilderMesytec::debug() const {
  std::stringstream ss;
  ss << "  =====================================================\n";
  ss << "  ========           Mesytec Builder           ========\n";
  ss << "  =====================================================\n";

  ss << "  Geometry mappings:\n";
  ss << digital_geometry_.debug("  ") << "\n";
  ss << "  Spoof high time (vmmr16):"
     << (vmmr16Parser_.spoof_high_time() ? "YES" : "no") << "\n";

  return ss.str();
}

void BuilderMesytec::parse(Buffer<uint8_t> buffer) {

  stats_discarded_bytes += sis3153parser_.parse(Buffer<uint8_t>(buffer));

  for (const auto &b : sis3153parser_.buffers) {

    stats_discarded_bytes += vmmr16Parser_.parse(b);

    if (vmmr16Parser_.converted_data.empty())
      continue;

    stats_trigger_count = vmmr16Parser_.trigger_count();

    if (dumpfile_) {
      dumpfile_->push(vmmr16Parser_.converted_data);
    }

    for (const auto &r : vmmr16Parser_.converted_data) {
      if (r.external_trigger) {
        hit_.plane = external_trigger_plane;
        hit_.coordinate = 0;
        hit_.weight = 0;
      } else if (digital_geometry_.isWire(r.bus, r.channel)) {
        hit_.weight = digital_geometry_.rescale(r.bus, r.channel, r.adc);
        if (!digital_geometry_.is_valid(r.bus, r.channel, hit_.weight)) {
          stats_readout_filter_rejects++;
          continue;
        }
        hit_.coordinate = digital_geometry_.wire(r.bus, r.channel);
        hit_.plane = wire_plane;
      } else if (digital_geometry_.isGrid(r.bus, r.channel)) {
        hit_.weight = digital_geometry_.rescale(r.bus, r.channel, r.adc);
        if (!digital_geometry_.is_valid(r.bus, r.channel, hit_.weight)) {
          stats_readout_filter_rejects++;
          continue;
        }
        hit_.coordinate = digital_geometry_.grid(r.bus, r.channel);
        hit_.plane = grid_plane;
      } else {
        XTRACE(PROCESS, DEB, "Bad geometry %s", r.debug().c_str());
        stats_digital_geom_errors++;
        continue;
      }

      hit_.time = r.total_time;
      ConvertedData.push_back(hit_);
    }

  }
}

}
