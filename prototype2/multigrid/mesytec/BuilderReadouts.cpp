/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/BuilderReadouts.h>
#include <common/TimeString.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

BuilderReadouts::BuilderReadouts(const DigitalGeometry &geometry,
                                 std::string dump_dir)
    : digital_geometry_(geometry) {
  if (!dump_dir.empty()) {
    dumpfile_ = HitFile::create(dump_dir + "multigrid_hits_" + timeString(), 100);
  }
  parsed_data_.reserve(9000 / sizeof(Readout));
}

std::string BuilderReadouts::debug() const {
  std::stringstream ss;
  ss << "  ======================================================\n";
  ss << "  ========           Readouts Builder           ========\n";
  ss << "  ======================================================\n";

  ss << "  Geometry mappings:\n";
  ss << digital_geometry_.debug("  ") << "\n";

  return ss.str();
}

void BuilderReadouts::parse(Buffer<uint8_t> buffer) {

  size_t count = std::min(buffer.size / sizeof(Readout),
                          size_t(9000 / sizeof(Readout)));

  parsed_data_.resize(count);
  memcpy(parsed_data_.data(), buffer.address, count * sizeof(Readout));

  //stats_trigger_count = vmmr16Parser.trigger_count();

  build(parsed_data_);

  if (dumpfile_) {
    dumpfile_->push(ConvertedData);
  }
}

void BuilderReadouts::build(const std::vector<Readout>& readouts) {
  stats_readouts_total += readouts.size();
  for (const auto &r : readouts) {
    if (r.external_trigger) {
      hit_.plane = external_trigger_plane;
      hit_.coordinate = 0;
      hit_.weight = 0;
    } else if (digital_geometry_.isWire(r.bus, 0, r.channel)) {
      hit_.weight = digital_geometry_.rescale(r.bus, 0, r.channel, r.adc);
      if (!digital_geometry_.is_valid(r.bus, 0, r.channel, hit_.weight)) {
        stats_readout_filter_rejects++;
        continue;
      }
      hit_.coordinate = digital_geometry_.wire(r.bus, 0, r.channel);
      hit_.plane = wire_plane;
    } else if (digital_geometry_.isGrid(r.bus, 0, r.channel)) {
      hit_.weight = digital_geometry_.rescale(r.bus,0,  r.channel, r.adc);
      if (!digital_geometry_.is_valid(r.bus, 0, r.channel, hit_.weight)) {
        stats_readout_filter_rejects++;
        continue;
      }
      hit_.coordinate = digital_geometry_.grid(r.bus, 0, r.channel);
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
