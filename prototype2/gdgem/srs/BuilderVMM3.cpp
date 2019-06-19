/** Copyright (C) 2016-2018 European Spallation Source ERIC */

#include <gdgem/srs/BuilderVMM3.h>
#include <common/clustering/GapClusterer.h>
#include <common/TimeString.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

namespace Gem {

BuilderVMM3::BuilderVMM3(SRSTime time_intepreter,
                         SRSMappings digital_geometry,
                         uint16_t adc_threshold,
                         std::string dump_dir,
                         std::shared_ptr<CalibrationFile> calfile)
                         : parser_(1500)
                         , time_intepreter_(time_intepreter)
                         , digital_geometry_(digital_geometry)
                         , adc_threshold_ (adc_threshold) {
  assert(calfile != nullptr);
  calfile_ = calfile;

  if (!dump_dir.empty()) {
    readout_file_ = ReadoutFile::create(dump_dir + "gdgem_readouts_" + timeString(), 1000);
  }
}

void BuilderVMM3::process_buffer(char *buf, size_t size) {
  parser_.receive(buf, size);
  const auto &parser_stats = parser_.stats;
  stats.parser_fc_seq_errors += parser_stats.rxSeqErrors;
  stats.parser_bad_frames += parser_stats.badFrames;
  stats.parser_good_frames += parser_stats.goodFrames;
  stats.parser_error_bytes += parser_stats.errors;
  stats.parser_readouts += parser_stats.readouts;

  if (!parser_.stats.readouts) {
    XTRACE(PROCESS, DEB, "No readouts after parse");
    return;
  }
  XTRACE(PROCESS, DEB, "Readouts after parse: %u", parser_.stats.readouts);

  //field fec id starts at 1
  readout.fec = parser_.parserData.fecId;
  for (unsigned int i = 0; i < parser_.stats.readouts; i++) {
    auto &d = parser_.data[i];
    if (d.hasDataMarker) {
      // \todo should these be functions of SRSTime?
      readout.srs_timestamp = (
          static_cast<uint64_t>(d.fecTimeStamp) * SRSTime::internal_SRS_clock_period_ns
              + static_cast<uint64_t>(d.triggerOffset) * time_intepreter_.trigger_period_ns()
      );

      readout.chip_id = d.vmmid;
      readout.channel = d.chno;
      readout.bcid = d.bcid;
      readout.tdc = d.tdc;
      readout.adc = d.adc;
      readout.over_threshold = (d.overThreshold != 0);
      auto calib = calfile_->getCalibration(readout.fec, readout.chip_id, readout.channel);
      // \todo does this really need to be a floating point value?
      readout.chiptime = static_cast<float>(time_intepreter_.chip_time_ns(d.bcid, d.tdc,
          calib.offset, calib.slope));

      // \todo what if chiptime is negative?

      XTRACE(PROCESS, DEB,
             "srs/vmm timestamp: srs: 0x%08x, bc: 0x%08x, tdc: 0x%08x",
             readout.srs_timestamp, d.bcid, d.tdc);
      XTRACE(PROCESS, DEB, "srs/vmm chip: %d, channel: %d",
             readout.chip_id, d.chno);

      if (readout_file_) {
        readout_file_->push(readout);
      }

      hit.plane = digital_geometry_.get_plane(readout);
      hit.coordinate = digital_geometry_.get_strip(readout);
      hit.weight = readout.adc;
      hit.time = readout.srs_timestamp;
      if (readout.chiptime >= 0)
        hit.time += static_cast<uint64_t>(readout.chiptime);
      else
        hit.time -= static_cast<uint64_t>(-readout.chiptime);

      if ((hit.plane != 0) && (hit.plane != 1)) {
        stats.geom_errors++;
        XTRACE(PROCESS, DEB, "Bad SRS mapping (plane) -- fec=%d, chip=%d",
               readout.fec, readout.chip_id);
        continue;
      }

      if (hit.coordinate == Hit::InvalidCoord) {
        stats.geom_errors++;
        XTRACE(PROCESS, DEB, "Bad SRS mapping (coordinate) -- fec=%d, chip=%d",
            readout.fec, readout.chip_id);
        continue;
      }

      if (!readout.over_threshold && (hit.weight < adc_threshold_)) {
        stats.adc_rejects++;
        XTRACE(PROCESS, DEB, "Below ADC threshold  adc=%d", hit.weight);
        continue;
      }


      if (hit.weight == 0) {
        XTRACE(PROCESS, WAR,
            "Accepted readout with adc=0, may distort uTPC results, hit=%s", hit.debug().c_str());
        // \todo What to do? Cannot be 0 for CoM in uTPC. Reject?
        stats.adc_zero++;
        hit.weight = 1;
      }


      if (hit.plane == 1) {
        hit_buffer_y.emplace_back(hit);
      }

      if (hit.plane == 0) {
        hit_buffer_x.emplace_back(hit);
      }

    } else {
      LOG(PROCESS, Sev::Warning, "No data marker in hit (increment counter?)");
    }
  }

}

}
