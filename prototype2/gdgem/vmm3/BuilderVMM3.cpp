/** Copyright (C) 2016-2018 European Spallation Source ERIC */

#include <gdgem/vmm3/BuilderVMM3.h>
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
                         SRSMappings geometry_interpreter,
                         uint16_t adc_threshold,
                         std::string dump_dir,
                         std::shared_ptr<CalibrationFile> calfile)
                         : parser_(1500)
                         , time_intepreter_(time_intepreter)
                         , geometry_interpreter_(geometry_interpreter)
                         , adc_threshold_ (adc_threshold) {
  assert(calfile != nullptr);
  calfile_ = calfile;

  if (!dump_dir.empty()) {
    readout_file_ = ReadoutFile::create(dump_dir + "gdgem_readouts_" + timeString(), 1000);
  }
}

AbstractBuilder::ResultStats BuilderVMM3::process_buffer(char *buf, size_t size) {
  geom_errors = 0;
  below_adc_threshold = 0;
  parser_.receive(buf, size);
  if (!parser_.stats.hits) {
    LOG(PROCESS, Sev::Debug, "No readouts after parse");
    auto &stats = parser_.stats;
    return AbstractBuilder::ResultStats(stats.hits, stats.errors,
                                        geom_errors, below_adc_threshold,
                                        stats.rxSeqErrors, stats.badFrames,
                                        stats.goodFrames);
  }
  LOG(PROCESS, Sev::Debug, "Readouts after parse: {}", parser_.stats.hits);

//	uint32_t udp_timestamp_ns = parser_.srsHeader.udpTimeStamp
//			* time_intepreter_.internal_clock_period_ns();

  //field fec id starts at 1
  readout.fec = parser_.parserData.fecId;
  for (unsigned int i = 0; i < parser_.stats.hits; i++) {
    auto &d = parser_.data[i];
    if (d.hasDataMarker) {
      // \todo should these be functions of SRSTime?
      readout.srs_timestamp = (
          static_cast<uint64_t>(d.fecTimeStamp) * static_cast<uint64_t>(time_intepreter_.internal_clock_period_ns())
              + static_cast<uint64_t>(d.triggerOffset) * static_cast<uint64_t>(time_intepreter_.trigger_period_ns())
      );

      readout.chip_id = d.vmmid;
      readout.channel = d.chno;
      readout.bcid = d.bcid;
      readout.tdc = d.tdc;
      readout.adc = d.adc;
      readout.over_threshold = (d.overThreshold != 0);
      auto calib = calfile_->getCalibration(readout.fec, readout.chip_id, readout.channel);
      // \todo does this really need to be a floating point value?
      readout.chiptime = time_intepreter_.chip_time_ns(d.bcid, d.tdc, calib.offset, calib.slope);

      // \todo what if chiptime is negative?

      XTRACE(PROCESS, DEB,
             "srs/vmm timestamp: srs: 0x%08x, bc: 0x%08x, tdc: 0x%08x",
             readout.srs_timestamp, d.bcid, d.tdc);
      XTRACE(PROCESS, DEB, "srs/vmm chip: %d, channel: %d",
             readout.chip_id, d.chno);

      if (readout_file_) {
        readout_file_->push(readout);
      }

      hit.plane = geometry_interpreter_.get_plane(readout);
      hit.coordinate = geometry_interpreter_.get_strip(readout);
      hit.weight = readout.adc;
      hit.time = readout.srs_timestamp + static_cast<uint64_t>(readout.chiptime);

      if ((hit.plane != 0) && (hit.plane != 1)) {
        // \todo make a counter for this in the pipeline, this is digital geometry
        geom_errors++;
        LOG(PROCESS, Sev::Debug, "Bad SRS mapping (plane) -- fec={}, chip={}",
               readout.fec, readout.chip_id);
        continue;
      }

      if (hit.coordinate == NMX_INVALID_GEOM_ID) {
        // \todo make a counter for this in the pipeline, this is digital geometry
        geom_errors++;
        LOG(PROCESS, Sev::Debug, "Bad SRS mapping (coordinate) -- fec={}, chip={}",
            readout.fec, readout.chip_id);
        continue;
      }

      if (!readout.over_threshold && (hit.weight < adc_threshold_)) {
        // \todo make a counter for this in the pipeline
        below_adc_threshold++;
        LOG(PROCESS, Sev::Debug, "Below ADC threshold  adc={}", hit.weight);
        continue;
      }


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

    } else {
      LOG(PROCESS, Sev::Warning, "No data marker in hit (increment counter?)");
    }
  }

  auto &stats = parser_.stats;
  return AbstractBuilder::ResultStats(stats.hits, stats.errors,
                                      geom_errors,
                                      below_adc_threshold,
                                      stats.rxSeqErrors,
                                      stats.badFrames,
                                      stats.goodFrames);
}

}