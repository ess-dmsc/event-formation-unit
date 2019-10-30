/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/BuilderReadouts.h>
#include <common/TimeString.h>
#include <cstring>

#include <common/Trace.h>
#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
// #undef TRC_MASK
// #define TRC_MASK 0

namespace Gem {

BuilderReadouts::BuilderReadouts(SRSMappings digital_geometry,
    uint16_t adc_threshold,
    std::string dump_dir)
    : digital_geometry_(digital_geometry)
    , adc_threshold_(adc_threshold) {
  converted_data.reserve(9000 / sizeof(Readout));
  if (!dump_dir.empty()) {
    hit_file_ = HitFile::create(dump_dir + "gdgem_hits_" + timeString(), 1000);
  }
}

void BuilderReadouts::process_buffer(char *buf, size_t size) {
  size_t count = std::min(size / sizeof(Readout), size_t(9000 / sizeof(Readout)));

  converted_data.resize(count);
  memcpy(converted_data.data(), buf, count * sizeof(Readout));

  for (const auto &readout : converted_data) {
    hit.plane = digital_geometry_.get_plane(readout);
    hit.coordinate = digital_geometry_.get_strip(readout);
    hit.weight = readout.adc;
    hit.time = readout.srs_timestamp;
    XTRACE(DATA, DEB, "Readout: plane %u, coord %u, weight %u, time %u",
            hit.plane, hit.coordinate, hit.weight, hit.time);

    if (readout.chiptime >= 0)
      hit.time += static_cast<uint64_t>(readout.chiptime);
    else
      hit.time -= static_cast<uint64_t>(-readout.chiptime);

    if ((hit.plane != 0) && (hit.plane != 1)) {
      stats.geom_errors++;
      LOG(PROCESS, Sev::Debug, "Bad SRS mapping (plane) -- fec={}, chip={}",
          readout.fec, readout.chip_id);
      continue;
    }

    if (hit.coordinate == Hit::InvalidCoord) {
      stats.geom_errors++;
      LOG(PROCESS, Sev::Debug, "Bad SRS mapping (coordinate) -- fec={}, chip={}",
          readout.fec, readout.chip_id);
      continue;
    }

    if (!readout.over_threshold && (hit.weight < adc_threshold_)) {
      stats.adc_rejects++;
      LOG(PROCESS, Sev::Debug, "Below ADC threshold  adc={}", hit.weight);
      continue;
    }

    if (hit_file_) {
      hit_file_->push(hit);
    }

    if (hit.weight == 0) {
//        LOG(PROCESS, Sev::Warning,
//            "Accepted readout with adc=0, may distort uTPC results, hit={}",
//            hit.to_string());
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
