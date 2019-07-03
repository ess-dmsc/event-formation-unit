/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/BuilderReadouts.h>
#include <common/TimeString.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

BuilderReadouts::BuilderReadouts(const DetectorMapping &geometry,
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

  ss << "  Mappings:\n";
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

void BuilderReadouts::build(const std::vector<Readout> &readouts) {
  stats_readouts_total += readouts.size();
  for (const auto &r : readouts) {
    if (r.external_trigger) {
      hit_.plane = Hit::PulsePlane;
      hit_.coordinate = Hit::InvalidCoord;
      hit_.weight = 0;
      hit_.time = r.total_time;
      ConvertedData.push_back(hit_);
      continue;
    }

    bool good = digital_geometry_.map(hit_, r.bus, r.channel, r.adc);
    if (hit_.plane == Hit::InvalidPlane) {
      XTRACE(PROCESS, DEB, "Bad mappings for %s", r.debug().c_str());
      stats_digital_geom_errors++;
      continue;
    }

    if (!good) {
      stats_readout_filter_rejects++;
      continue;
    }

    hit_.time = r.total_time;
    ConvertedData.push_back(hit_);
  }
}


}
